/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
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

# include  "vvp_priv.h"
# include  <string.h>
# include  <assert.h>
# include  <stdlib.h>

#ifdef __MINGW32__ /* MinGW has inconsistent %p output. */
#define snprintf _snprintf
#endif

static int show_statement(ivl_statement_t net, ivl_scope_t sscope);

unsigned local_count = 0;
unsigned thread_count = 0;

static unsigned transient_id = 0;

/*
 * This file includes the code needed to generate VVP code for
 * processes. Scopes are already declared, we generate here the
 * executable code for the processes.
 */


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

static void set_to_lvariable(ivl_lval_t lval,
			     unsigned bit, unsigned wid)
{
      ivl_signal_t sig  = ivl_lval_sig(lval);
      ivl_expr_t part_off_ex = ivl_lval_part_off(lval);
      unsigned long part_off = 0;

	/* Although Verilog doesn't support it, we'll handle
	   here the case of an l-value part select of an array
	   word if the address is constant. */
      ivl_expr_t word_ix = ivl_lval_idx(lval);
      unsigned long use_word = 0;

      if (part_off_ex == 0) {
	    part_off = 0;
      } else if (number_is_immediate(part_off_ex, IMM_WID, 0) &&
                 !number_is_unknown(part_off_ex)) {
	    part_off = get_number_immediate(part_off_ex);
	    part_off_ex = 0;
      }

	/* If the word index is a constant expression, then evaluate
	   it to select the word, and pay no further heed to the
	   expression itself. */
      if (word_ix && number_is_immediate(word_ix, IMM_WID, 0)) {
	    assert(! number_is_unknown(word_ix));
	    use_word = get_number_immediate(word_ix);
	    word_ix = 0;
      }

      if (ivl_lval_mux(lval))
	    part_off_ex = ivl_lval_mux(lval);

      if (part_off_ex && ivl_signal_dimensions(sig) == 0) {
	    unsigned skip_set = transient_id++;

	      /* There is a mux expression, so this must be a write to
		 a bit-select l-val. Presumably, the x0 index register
		 has been loaded wit the result of the evaluated
		 part select base expression. */
	    assert(!word_ix);

	    draw_eval_expr_into_integer(part_off_ex, 0);
	    fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);

	    fprintf(vvp_out, "    %%set/x0 v%p_%lu, %u, %u;\n",
		    sig, use_word, bit, wid);
	    fprintf(vvp_out, "t_%u ;\n", skip_set);
	      /* save_signal width of 0 CLEARS the signal from the
	         lookaside. */
	    save_signal_lookaside(bit, sig, use_word, 0);

      } else if (part_off_ex && ivl_signal_dimensions(sig) > 0) {

	      /* Here we have a part select write into an array word. */
	    unsigned skip_set = transient_id++;
	    if (word_ix) {
		  draw_eval_expr_into_integer(word_ix, 3);
		  fprintf(vvp_out, "   %%jmp/1 t_%u, 4;\n", skip_set);
	    } else {
		  fprintf(vvp_out, "   %%ix/load 3, %lu, 0;\n", use_word);
	    }
	    draw_eval_expr_into_integer(part_off_ex, 1);
	    fprintf(vvp_out, "   %%jmp/1 t_%u, 4;\n", skip_set);
	    fprintf(vvp_out, "   %%set/av v%p, %u, %u;\n",
		    sig, bit, wid);
	    fprintf(vvp_out, "t_%u ;\n", skip_set);

      } else if ((part_off>0 || ivl_lval_width(lval)!=ivl_signal_width(sig))
		 && ivl_signal_dimensions(sig) > 0) {

	      /* Here we have a part select write into an array word. */
	    unsigned skip_set = transient_id++;
	    if (word_ix) {
		  draw_eval_expr_into_integer(word_ix, 3);
		  fprintf(vvp_out, "   %%jmp/1 t_%u, 4;\n", skip_set);
	    } else {
		  fprintf(vvp_out, "   %%ix/load 3, %lu, 0;\n", use_word);
	    }
	    fprintf(vvp_out, "   %%ix/load 1, %lu, 0;\n", part_off);
	    fprintf(vvp_out, "   %%set/av v%p, %u, %u;\n",
		    sig, bit, wid);
	    if (word_ix) /* Only need this label if word_ix is set. */
		  fprintf(vvp_out, "t_%u ;\n", skip_set);

      } else if (part_off>0 || ivl_lval_width(lval)!=ivl_signal_width(sig)) {
	      /* There is no mux expression, but a constant part
		 offset. Load that into index x0 and generate a
		 vector set instruction. */
	    assert(ivl_lval_width(lval) == wid);

	      /* If the word index is a constant, then we can write
	         directly to the word and save the index calculation. */
	    if (word_ix == 0) {
		  fprintf(vvp_out, "    %%ix/load 0, %lu, 0;\n", part_off);
		  fprintf(vvp_out, "    %%set/x0 v%p_%lu, %u, %u;\n",
		          sig, use_word, bit, wid);

	    } else {
		  unsigned skip_set = transient_id++;
		  unsigned index_reg = 3;
		  draw_eval_expr_into_integer(word_ix, index_reg);
		  fprintf(vvp_out, "   %%jmp/1 t_%u, 4;\n", skip_set);
		  fprintf(vvp_out, "   %%ix/load 1, %lu, 0;\n", part_off);
		  fprintf(vvp_out, "   %%set/av v%p, %u, %u;\n",
			  sig, bit, wid);
		  fprintf(vvp_out, "t_%u ;\n", skip_set);
	    }
	      /* save_signal width of 0 CLEARS the signal from the
	         lookaside. */
	    save_signal_lookaside(bit, sig, use_word, 0);

      } else if (ivl_signal_dimensions(sig) > 0) {

	      /* If the word index is a constant, then we can write
	         directly to the word and save the index calculation. */
	    if (word_ix == 0) {
		  if (use_word < ivl_signal_array_count(sig)) {
			fprintf(vvp_out, "   %%ix/load 1, 0, 0;\n");
			fprintf(vvp_out, "   %%ix/load 3, %lu, 0;\n", use_word);
			fprintf(vvp_out, "   %%set/av v%p, %u, %u;\n",
				sig, bit, wid);
		  } else {
			fprintf(vvp_out, " ; %%set/v v%p_%lu, %u, %u "
				"OUT OF BOUNDS\n", sig, use_word, bit, wid);
		  }

	    } else {
		  unsigned skip_set = transient_id++;
		  unsigned index_reg = 3;
		  draw_eval_expr_into_integer(word_ix, index_reg);
		  fprintf(vvp_out, "   %%jmp/1 t_%u, 4;\n", skip_set);
		  fprintf(vvp_out, "   %%ix/load 1, 0, 0;\n");
		  fprintf(vvp_out, "   %%set/av v%p, %u, %u;\n",
			  sig, bit, wid);
		  fprintf(vvp_out, "t_%u ;\n", skip_set);
	    }
	      /* save_signal width of 0 CLEARS the signal from the
	         lookaside. */
	    save_signal_lookaside(bit, sig, use_word, 0);


      } else {
	    fprintf(vvp_out, "    %%set/v v%p_%lu, %u, %u;\n",
		    sig, use_word, bit, wid);
	      /* save_signal width of 0 CLEARS the signal from the
	         lookaside. */
	    save_signal_lookaside(bit, sig, use_word, 0);

      }
}

static void assign_to_array_word(ivl_signal_t lsig, ivl_expr_t word_ix,
				 unsigned bit, uint64_t delay, ivl_expr_t dexp,
				 ivl_expr_t part_off_ex, unsigned width,
				 unsigned nevents)
{
      unsigned skip_assign = transient_id++;

      unsigned long part_off = 0;
      if (part_off_ex == 0) {
	    part_off = 0;
      } else if (number_is_immediate(part_off_ex, IMM_WID, 0)) {
	    assert(! number_is_unknown(part_off_ex));
	    part_off = get_number_immediate(part_off_ex);
	    part_off_ex = 0;
      }

	/* This code is common to all the different types of array delays. */
      if (number_is_immediate(word_ix, IMM_WID, 0)) {
	    assert(! number_is_unknown(word_ix));
	    fprintf(vvp_out, "    %%ix/load 3, %lu, 0; address\n",
	                     get_number_immediate(word_ix));
      } else {
	      /* Calculate array word index into index register 3 */
	    draw_eval_expr_into_integer(word_ix, 3);
	      /* Skip assignment if word expression is not defined. */
	    fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_assign);
      }
	/* Store expression width into index word 0 */
      fprintf(vvp_out, "    %%ix/load 0, %u, 0; word width\n", width);
      if (part_off_ex) {
	    draw_eval_expr_into_integer(part_off_ex, 1);
	      /* If the index expression has XZ bits, skip the assign. */
	    fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_assign);
      } else {
	      /* Store word part select into index 1 */
	    fprintf(vvp_out, "    %%ix/load 1, %lu, 0; part off\n", part_off);
      }

      if (dexp != 0) {
	      /* Calculated delay... */
	    int delay_index = allocate_word();
	    draw_eval_expr_into_integer(dexp, delay_index);
	    fprintf(vvp_out, "    %%assign/av/d v%p, %d, %u;\n", lsig,
	                     delay_index, bit);
	    clr_word(delay_index);
      } else if (nevents != 0) {
	      /* Event control delay... */
	    fprintf(vvp_out, "    %%assign/av/e v%p, %u;\n", lsig, bit);
      } else {
	      /* Constant delay... */
	    unsigned long low_d = delay % UINT64_C(0x100000000);
	    unsigned long hig_d = delay / UINT64_C(0x100000000);

	      /*
	       * The %assign can only take a 32 bit delay. For a larger
	       * delay we need to put it into an index register.
	       */
	    if (hig_d != 0) {
		  int delay_index = allocate_word();
		  fprintf(vvp_out, "    %%ix/load %d, %lu, %lu;\n",
		          delay_index, low_d, hig_d);
		  fprintf(vvp_out, "    %%assign/av/d v%p, %d, %u;\n", lsig,
		  delay_index, bit);
		  clr_word(delay_index);
	    } else {
		  fprintf(vvp_out, "    %%assign/av v%p, %lu, %u;\n",
		          lsig, low_d, bit);
	    }
      }

      fprintf(vvp_out, "t_%u ;\n", skip_assign);
      if (nevents != 0) fprintf(vvp_out, "    %%evctl/c;\n");

      clear_expression_lookaside();
}

static void assign_to_lvector(ivl_lval_t lval, unsigned bit,
			      uint64_t delay, ivl_expr_t dexp,
			      unsigned width, unsigned nevents)
{
      ivl_signal_t sig = ivl_lval_sig(lval);
      ivl_expr_t part_off_ex = ivl_lval_part_off(lval);
      unsigned long part_off = 0;

      ivl_expr_t word_ix = ivl_lval_idx(lval);
      const unsigned long use_word = 0;

      if (ivl_signal_dimensions(sig) > 0) {
	    assert(word_ix);
	    assign_to_array_word(sig, word_ix, bit, delay, dexp, part_off_ex,
	                         width, nevents);
	    return;
      }

      if (part_off_ex == 0) {
	    part_off = 0;
      } else if (number_is_immediate(part_off_ex, IMM_WID, 0)) {
	    assert(! number_is_unknown(part_off_ex));
	    part_off = get_number_immediate(part_off_ex);
	    part_off_ex = 0;
      }

      if (ivl_lval_mux(lval))
	    part_off_ex = ivl_lval_mux(lval);

      unsigned long low_d = delay % UINT64_C(0x100000000);
      unsigned long hig_d = delay / UINT64_C(0x100000000);

      if (part_off_ex) {
	    unsigned skip_assign = transient_id++;
	    if (dexp != 0) {
		    /* Calculated delay... */
		  int delay_index = allocate_word();
		  draw_eval_expr_into_integer(dexp, delay_index);
		  draw_eval_expr_into_integer(part_off_ex, 1);
		    /* If the index expression has XZ bits, skip the assign. */
		  fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_assign);
		  fprintf(vvp_out, "    %%ix/load 0, %u, 0;\n", width);
		  fprintf(vvp_out, "    %%assign/v0/x1/d v%p_%lu, %d, %u;\n",
		          sig, use_word, delay_index, bit);
		  fprintf(vvp_out, "t_%u ;\n", skip_assign);
		  clr_word(delay_index);
	    } else if (nevents != 0) {
		    /* Event control delay... */
		  draw_eval_expr_into_integer(part_off_ex, 1);
		    /* If the index expression has XZ bits, skip the assign. */
		  fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_assign);
		  fprintf(vvp_out, "    %%ix/load 0, %u, 0;\n", width);
		  fprintf(vvp_out, "    %%assign/v0/x1/e v%p_%lu, %u;\n",
		          sig, use_word, bit);
		  fprintf(vvp_out, "t_%u ;\n", skip_assign);
		  fprintf(vvp_out, "    %%evctl/c;\n");
	    } else {
		    /* Constant delay... */
		  draw_eval_expr_into_integer(part_off_ex, 1);
		    /* If the index expression has XZ bits, skip the assign. */
		  fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_assign);
		  fprintf(vvp_out, "    %%ix/load 0, %u, 0;\n", width);
		    /*
		     * The %assign can only take a 32 bit delay. For a larger
		     * delay we need to put it into an index register.
		     */
		  if (hig_d != 0) {
			int delay_index = allocate_word();
			fprintf(vvp_out, "    %%ix/load %d, %lu, %lu;\n",
			        delay_index, low_d, hig_d);
			fprintf(vvp_out,
			        "    %%assign/v0/x1/d v%p_%lu, %d, %u;\n",
			        sig, use_word, delay_index, bit);
			clr_word(delay_index);
		  } else {
			fprintf(vvp_out,
			        "    %%assign/v0/x1 v%p_%lu, %lu, %u;\n",
			        sig, use_word, low_d, bit);
		  }
		  fprintf(vvp_out, "t_%u ;\n", skip_assign);
	    }

      } else if (part_off>0 || ivl_lval_width(lval)!=ivl_signal_width(sig)) {
	      /* There is no mux expression, but a constant part
		 offset. Load that into index x1 and generate a
		 single-bit set instruction. */
	    assert(ivl_lval_width(lval) == width);

	    if (dexp != 0) {
		    /* Calculated delay... */
		  int delay_index = allocate_word();
		  draw_eval_expr_into_integer(dexp, delay_index);
		  fprintf(vvp_out, "    %%ix/load 0, %u, 0;\n", width);
		  fprintf(vvp_out, "    %%ix/load 1, %lu, 0;\n", part_off);
		  fprintf(vvp_out, "    %%assign/v0/x1/d v%p_%lu, %d, %u;\n",
			  sig, use_word, delay_index, bit);
		  clr_word(delay_index);
	    } else if (nevents != 0) {
		    /* Event control delay... */
		  fprintf(vvp_out, "    %%ix/load 0, %u, 0;\n", width);
		  fprintf(vvp_out, "    %%ix/load 1, %lu, 0;\n", part_off);
		  fprintf(vvp_out, "    %%assign/v0/x1/e v%p_%lu, %u;\n",
			  sig, use_word, bit);
	    } else {
		    /* Constant delay... */
		  fprintf(vvp_out, "    %%ix/load 0, %u, 0;\n", width);
		  fprintf(vvp_out, "    %%ix/load 1, %lu, 0;\n", part_off);
		    /*
		     * The %assign can only take a 32 bit delay. For a larger
		     * delay we need to put it into an index register.
		     */
		  if (hig_d != 0) {
			int delay_index = allocate_word();
			fprintf(vvp_out, "    %%ix/load %d, %lu, %lu;\n",
			        delay_index, low_d, hig_d);
			fprintf(vvp_out,
			        "    %%assign/v0/x1/d v%p_%lu, %d, %u;\n",
			        sig, use_word, delay_index, bit);
			clr_word(delay_index);
		  } else {
			fprintf(vvp_out,
			        "    %%assign/v0/x1 v%p_%lu, %lu, %u;\n",
			        sig, use_word, low_d, bit);
		  }
	    }

      } else if (dexp != 0) {
	      /* Calculated delay... */
	    int delay_index = allocate_word();
	    draw_eval_expr_into_integer(dexp, delay_index);
	    fprintf(vvp_out, "    %%ix/load 0, %u, 0;\n", width);
	    fprintf(vvp_out, "    %%assign/v0/d v%p_%lu, %d, %u;\n",
		    sig, use_word, delay_index, bit);
	    clr_word(delay_index);
      } else if (nevents != 0) {
	      /* Event control delay... */
	    fprintf(vvp_out, "    %%ix/load 0, %u, 0;\n", width);
	    fprintf(vvp_out, "    %%assign/v0/e v%p_%lu, %u;\n",
		    sig, use_word, bit);
      } else {
	      /* Constant delay... */
	    fprintf(vvp_out, "    %%ix/load 0, %u, 0;\n", width);
	      /*
	       * The %assign can only take a 32 bit delay. For a larger
	       * delay we need to put it into an index register.
	       */
	    if (hig_d != 0) {
		  int delay_index = allocate_word();
		  fprintf(vvp_out, "    %%ix/load %d, %lu, %lu;\n",
		          delay_index, low_d, hig_d);
		  fprintf(vvp_out, "    %%assign/v0/d v%p_%lu, %d, %u;\n",
		          sig, use_word, delay_index, bit);
		  clr_word(delay_index);
	    } else {
		  fprintf(vvp_out, "    %%assign/v0 v%p_%lu, %lu, %u;\n",
		          sig, use_word, low_d, bit);
	    }
      }
}


/*
 * This is a private function to generate %set code for the
 * statement. At this point, the r-value is evaluated and stored in
 * the res vector, I just need to generate the %set statements for the
 * l-values of the assignment.
 */
static void set_vec_to_lval(ivl_statement_t net, struct vector_info res)
{
      ivl_lval_t lval;

      unsigned wid = res.wid;
      unsigned lidx;
      unsigned cur_rbit = 0;

      for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
	    unsigned bidx;
	    unsigned bit_limit = wid - cur_rbit;

	    lval = ivl_stmt_lval(net, lidx);

	      /* Reduce bit_limit to the width of this l-value. */
	    if (bit_limit > ivl_lval_width(lval))
		  bit_limit = ivl_lval_width(lval);

	      /* This is the address within the larger r-value of the
		 bit that this l-value takes. */
	    bidx = res.base < 4? res.base : (res.base+cur_rbit);

	    set_to_lvariable(lval, bidx, bit_limit);

	      /* Now we've consumed this many r-value bits for the
		 current l-value. */
	    cur_rbit += bit_limit;
      }
}

static int show_stmt_alloc(ivl_statement_t net)
{
      ivl_scope_t scope = ivl_stmt_call(net);

      fprintf(vvp_out, "    %%alloc S_%p;\n", scope);
      return 0;
}

static int show_stmt_assign_vector(ivl_statement_t net)
{
      ivl_expr_t rval = ivl_stmt_rval(net);

	/* Handle the special case that the expression is a real
	   value. Evaluate the real expression, then convert the
	   result to a vector. Then store that vector into the
	   l-value. */
      if (ivl_expr_value(rval) == IVL_VT_REAL) {
	    int word = draw_eval_real(rval);
	      /* This is the accumulated with of the l-value of the
		 assignment. */
	    unsigned wid = ivl_stmt_lwidth(net);

	    struct vector_info vec;

	    vec.base = allocate_vector(wid);
	    vec.wid = wid;

	    if (vec.base == 0) {
		  fprintf(stderr, "%s:%u: vvp.tgt error: "
			  "Unable to allocate %u thread bits for "
			  "r-value expression.\n", ivl_expr_file(rval),
			  ivl_expr_lineno(rval), wid);
		  vvp_errors += 1;
	    }

	    fprintf(vvp_out, "    %%cvt/vr %u, %d, %u;\n",
		    vec.base, word, vec.wid);

	    clr_word(word);

	    set_vec_to_lval(net, vec);

	    clr_vector(vec);
	    return 0;
      }


      { struct vector_info res = draw_eval_expr(rval, 0);
        set_vec_to_lval(net, res);
	if (res.base > 3)
	      clr_vector(res);
      }


      return 0;
}

/*
 * This function assigns a value to a real .variable. This is destined
 * for /dev/null when typed ivl_signal_t takes over all the real
 * variable support.
 */
static int show_stmt_assign_sig_real(ivl_statement_t net)
{
      int res;
      ivl_lval_t lval;
      ivl_signal_t var;

      res = draw_eval_real(ivl_stmt_rval(net));

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);
      var = ivl_lval_sig(lval);
      assert(var != 0);

      if (ivl_signal_dimensions(var) == 0) {
	    clr_word(res);
	    fprintf(vvp_out, "    %%set/wr v%p_0, %d;\n", var, res);
	    return 0;
      }

	// For now, only support 1-dimensional arrays.
      assert(ivl_signal_dimensions(var) == 1);

	// Calculate the word index into an index register
      ivl_expr_t word_ex = ivl_lval_idx(lval);
      int word_ix = allocate_word();
      draw_eval_expr_into_integer(word_ex, word_ix);
	// Generate an assignment to write to the array.
      fprintf(vvp_out, "    %%set/ar v%p, %d, %d;\n", var, word_ix, res);

      clr_word(res);
      clr_word(word_ix);

      return 0;
}

static int show_stmt_assign(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_signal_t sig;

      lval = ivl_stmt_lval(net, 0);

      sig = ivl_lval_sig(lval);
      if (sig && (ivl_signal_data_type(sig) == IVL_VT_REAL)) {
	    return show_stmt_assign_sig_real(net);
      }

      return show_stmt_assign_vector(net);
}

/*
 * This function handles the case of non-blocking assign to word
 * variables such as real, i.e:
 *
 *     real foo;
 *     foo <= 1.0;
 *
 * In this case we know (by Verilog syntax) that there is only exactly
 * 1 l-value, the target identifier, so it should be relatively easy.
 */
static int show_stmt_assign_nb_real(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_signal_t sig;
      ivl_expr_t rval = ivl_stmt_rval(net);
      ivl_expr_t del  = ivl_stmt_delay_expr(net);
	/* variables for the selection of word from an array. */
      ivl_expr_t word_ix;
      unsigned long use_word = 0;
	/* thread address for a word value. */
      int word;
      uint64_t delay = 0;
      unsigned nevents = ivl_stmt_nevent(net);

	/* Must be exactly 1 l-value. */
      assert(ivl_stmt_lvals(net) == 1);

      lval = ivl_stmt_lval(net, 0);
      sig = ivl_lval_sig(lval);
      assert(sig);

      if (ivl_signal_dimensions(sig) > 0) {
	    word_ix = ivl_lval_idx(lval);
	    assert(word_ix);
	    assert(number_is_immediate(word_ix, IMM_WID, 0));
	    assert(! number_is_unknown(word_ix));
	    use_word = get_number_immediate(word_ix);
	    /* This method no longer works since variable arrays do not
	     * support <variable_id>_<idx> access any more. We need real
	     * array specific opcodes (%assign/ar, etc.). */
	    fprintf(stderr, "%s:%u: tgt-vvp sorry: non-blocking assignment "
	            "to a real array word is not supported in V0.9.\n",
	            ivl_expr_file(rval), ivl_expr_lineno(rval));
	    exit(1);
      }

      if (del && (ivl_expr_type(del) == IVL_EX_DELAY)) {
	    assert(number_is_immediate(del, 64, 0));
	    delay = ivl_expr_delay_val(del);
	    del = 0;
      }

	/* Evaluate the r-value */
      word = draw_eval_real(rval);

	/* We need to calculate the delay expression. */
      if (del) {
	    assert(nevents == 0);
	    int delay_index = allocate_word();
	    draw_eval_expr_into_integer(del, delay_index);
	    fprintf(vvp_out, "    %%assign/wr/d v%p_%lu, %d, %u;\n",
	            sig, use_word, delay_index, word);
	    clr_word(delay_index);
      } else if (nevents) {
	    fprintf(vvp_out, "    %%assign/wr/e v%p_%lu, %u;\n",
	            sig, use_word, word);
      } else {
	    unsigned long low_d = delay % UINT64_C(0x100000000);
	    unsigned long hig_d = delay / UINT64_C(0x100000000);

	      /*
	       * The %assign can only take a 32 bit delay. For a larger
	       * delay we need to put it into an index register.
	       */
	    if (hig_d != 0) {
		  int delay_index = allocate_word();
		  fprintf(vvp_out, "    %%ix/load %d, %lu, %lu;\n",
		          delay_index, low_d, hig_d);
		  fprintf(vvp_out, "    %%assign/wr/d v%p_%lu, %d, %u;\n",
		          sig, use_word, delay_index, word);
		  clr_word(delay_index);
	    } else {
		  fprintf(vvp_out, "    %%assign/wr v%p_%lu, %lu, %u;\n",
		          sig, use_word, low_d, word);
	    }
      }

      clr_word(word);

      return 0;
}

static int show_stmt_assign_nb(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_expr_t rval = ivl_stmt_rval(net);
      ivl_expr_t del  = ivl_stmt_delay_expr(net);
      ivl_signal_t sig;
      unsigned nevents = ivl_stmt_nevent(net);

	/* If we have an event control build the control structure. */
      if (nevents) {
	    assert(del == 0);

	    ivl_expr_t cnt = ivl_stmt_cond_expr(net);
	    unsigned long count = 1;
	    if (cnt && (ivl_expr_type(cnt) == IVL_EX_ULONG)) {
		  assert(number_is_immediate(cnt, IMM_WID, 0));
		  count = ivl_expr_uvalue(cnt);
		  cnt = 0;
	    }

	    char name[256];
	    if (nevents == 1) {
		  ivl_event_t ev = ivl_stmt_events(net, 0);
		  snprintf(name, sizeof(name), "E_%p", ev);
	    } else {
		  unsigned idx;
		  static unsigned int cascade_counter = 0;
		  ivl_event_t ev = ivl_stmt_events(net, 0);
		  fprintf(vvp_out, "Eassign_%u .event/or E_%p",
		                   cascade_counter, ev);

		  for (idx = 1;  idx < nevents;  idx += 1) {
			ev = ivl_stmt_events(net, idx);
			fprintf(vvp_out, ", E_%p", ev);
		  }
		  snprintf(name, sizeof(name), "Eassign_%u", cascade_counter);
		  cascade_counter += 1;
	    }

	    if (cnt) {
		  int count_index = allocate_word();
		  const char*type = ivl_expr_signed(cnt) ? "/s" : "";
		  draw_eval_expr_into_integer(cnt, count_index);
		  fprintf(vvp_out, "    %%evctl%s %s, %d;\n", type, name,
		                   count_index);
		  clr_word(count_index);
	    } else {
		  fprintf(vvp_out, "    %%evctl/i %s, %lu;\n", name, count);
	    }
      } else {
	    assert(ivl_stmt_cond_expr(net) == 0);
      }

      uint64_t delay = 0;

	/* Detect special cases that are handled elsewhere. */
      lval = ivl_stmt_lval(net,0);
      if ((sig = ivl_lval_sig(lval))) {
	    switch (ivl_signal_data_type(sig)) {
		case IVL_VT_REAL:
		  return show_stmt_assign_nb_real(net);
		default:
		  break;
	    }
      }

      if (del && (ivl_expr_type(del) == IVL_EX_DELAY)) {
	    assert(number_is_immediate(del, 64, 0));
	    delay = ivl_expr_delay_val(del);
	    del = 0;
      }


      { struct vector_info res;
	unsigned wid;
	unsigned lidx;
	unsigned cur_rbit = 0;
	  /* Handle the special case that the expression is a real
	     value. Evaluate the real expression, then convert the
	     result to a vector. */
	if (ivl_expr_value(rval) == IVL_VT_REAL) {
	      int word = draw_eval_real(rval);
	        /* This is the accumulated with of the l-value of the
		   assignment. */
	      wid = ivl_stmt_lwidth(net);

	      res.base = allocate_vector(wid);
	      res.wid = wid;

	      if (res.base == 0) {
		    fprintf(stderr, "%s:%u: vvp.tgt error: "
			    "Unable to allocate %u thread bits for "
			    "r-value expression.\n", ivl_expr_file(rval),
			    ivl_expr_lineno(rval), wid);
		    vvp_errors += 1;
	      }

	      fprintf(vvp_out, "    %%cvt/vr %u, %d, %u;\n",
		      res.base, word, res.wid);

	      clr_word(word);

	} else {
	      res = draw_eval_expr(rval, 0);
	      wid = res.wid;
	}

	for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
	      unsigned bit_limit = wid - cur_rbit;
	      unsigned bidx;

	      lval = ivl_stmt_lval(net, lidx);

	      if (bit_limit > ivl_lval_width(lval))
		    bit_limit = ivl_lval_width(lval);

	      bidx = res.base < 4? res.base : (res.base+cur_rbit);
	      assign_to_lvector(lval, bidx, delay, del, bit_limit, nevents);

	      cur_rbit += bit_limit;

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

      fprintf(vvp_out, "    %%fork t_%u, S_%p;\n",
	      sub_id, subscope);
      fprintf(vvp_out, "    %%jmp t_%u;\n", out_id);
	/* Change the compiling scope to be the named blocks scope. */
      fprintf(vvp_out, "    .scope S_%p;\n", subscope);
      fprintf(vvp_out, "t_%u ;\n", sub_id);

	/* The statement within the fork is in a new thread, so no
	  expression lookaside is valid. */
      clear_expression_lookaside();

      rc = show_stmt_block(net, subscope);
      fprintf(vvp_out, "    %%end;\n");
	/* Return to the previous scope. */
      fprintf(vvp_out, "    .scope S_%p;\n", scope);

      fprintf(vvp_out, "t_%u %%join;\n", out_id);
      clear_expression_lookaside();

      return rc;
}


static int show_stmt_case(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      ivl_expr_t expr = ivl_stmt_cond_expr(net);
      struct vector_info cond = draw_eval_expr(expr, 0);
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
		&& number_is_immediate(cex, 16, 0)) {

		  unsigned long imm = get_number_immediate(cex);

		  fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
			  cond.base, imm, cond.wid);
		  fprintf(vvp_out, "    %%jmp/1 T_%d.%d, 6;\n",
			  thread_count, local_base+idx);

		  continue;
	    }

	      /* Oh well, do this case the hard way. */

	    cvec = draw_eval_expr_wid(cex, cond.wid, STUFF_OK_RO);
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
	    rc += show_statement(cst, sscope);
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
	    rc += show_statement(cst, sscope);

	    fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count,
		    local_base+count);

      }


	/* The out of the case. */
      fprintf(vvp_out, "T_%d.%d ;\n",  thread_count, local_base+count);
      clear_expression_lookaside();

      return rc;
}

static int show_stmt_case_r(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      ivl_expr_t expr = ivl_stmt_cond_expr(net);
      int cond = draw_eval_real(expr);
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
	    int cvec;

	    if (cex == 0) {
		  default_case = idx;
		  continue;
	    }

	    cvec = draw_eval_real(cex);

	    fprintf(vvp_out, "    %%cmp/wr %d, %d;\n", cond, cvec);
	    fprintf(vvp_out, "    %%jmp/1 T_%d.%d, 4;\n",
		    thread_count, local_base+idx);

	      /* Done with the guard expression value. */
	    clr_word(cvec);
      }

	/* Done with the case expression. */
      clr_word(cond);

	/* Emit code for the case default. The above jump table will
	   fall through to this statement. */
      if (default_case < count) {
	    ivl_statement_t cst = ivl_stmt_case_stmt(net, default_case);
	    rc += show_statement(cst, sscope);
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
	    rc += show_statement(cst, sscope);

	    fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count,
		    local_base+count);

      }


	/* The out of the case. */
      fprintf(vvp_out, "T_%d.%d ;\n",  thread_count, local_base+count);

      return 0;
}

static void force_real_to_lval(ivl_statement_t net, int res)
{
      const char*command_name;
      ivl_lval_t lval;
      ivl_signal_t lsig;

      switch (ivl_statement_type(net)) {
	  case IVL_ST_CASSIGN:
	    command_name = "%cassign/wr";
	    break;
	  case IVL_ST_FORCE:
	    command_name = "%force/wr";
	    break;
	  default:
	    command_name = "ERROR";
	    assert(0);
	    break;
      }

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);
      lsig = ivl_lval_sig(lval);

      assert(ivl_lval_width(lval) == 1);
      assert(ivl_lval_part_off(lval) == 0);
      assert(ivl_lval_idx(lval) == 0);
	/* L-Value must be a signal: reg or wire */
      assert(lsig != 0);

      fprintf(vvp_out, "    %s v%p_0, %d;\n", command_name, lsig, res);

}

static void force_vector_to_lval(ivl_statement_t net, struct vector_info rvec)
{
      unsigned lidx;
      unsigned roff = 0;

      const char*command_name;
      const char*command_name_x0;

      switch (ivl_statement_type(net)) {
	  case IVL_ST_CASSIGN:
	    command_name = "%cassign/v";
	    command_name_x0 = "%cassign/x0";
	    break;
	  case IVL_ST_FORCE:
	    command_name = "%force/v";
	    command_name_x0 = "%force/x0";
	    break;
	  default:
	    command_name = "ERROR";
	    command_name_x0 = "ERROR";
	    assert(0);
	    break;
      }

      for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ; lidx += 1) {
	    ivl_lval_t lval = ivl_stmt_lval(net, lidx);
	    ivl_signal_t lsig = ivl_lval_sig(lval);

	    unsigned use_wid = ivl_lval_width(lval);
	    ivl_expr_t part_off_ex = ivl_lval_part_off(lval);
	    unsigned long part_off;
	    ivl_expr_t word_idx = ivl_lval_idx(lval);
	    unsigned long use_word = 0;

	    if (part_off_ex == 0) {
		  part_off = 0;
	    } else {
		  assert(number_is_immediate(part_off_ex, IMM_WID, 0));
		  assert(! number_is_unknown(part_off_ex));
		  part_off = get_number_immediate(part_off_ex);
	    }

	    if (word_idx != 0) {
		  assert(number_is_immediate(word_idx, IMM_WID, 0));
		  assert(! number_is_unknown(word_idx));
		  use_word = get_number_immediate(word_idx);
	    }

	      /* L-Value must be a signal: reg or wire */
	    assert(lsig != 0);

	    if (part_off != 0 || use_wid != ivl_signal_width(lsig)) {

		  command_name = command_name_x0;
		  fprintf(vvp_out, "    %%ix/load 0, %lu, 0;\n", part_off);

	    } else {
		    /* Do not support bit or part selects of l-values yet. */
		  assert(ivl_lval_mux(lval) == 0);
		  assert(ivl_lval_width(lval) == ivl_signal_width(lsig));

		  assert((roff + use_wid) <= rvec.wid);
	    }

	    fprintf(vvp_out, "    %s v%p_%lu, %u, %u;\n", command_name,
		    lsig, use_word, rvec.base+roff, use_wid);

	    if (rvec.base >= 4)
		  roff += use_wid;
      }
}

static void force_link_rval(ivl_statement_t net, ivl_expr_t rval)
{
      ivl_signal_t rsig;
      ivl_lval_t lval;
      ivl_signal_t lsig;
      const char*command_name;
      ivl_expr_t part_off_ex;

      ivl_expr_t lword_idx, rword_idx;
      unsigned long use_lword = 0, use_rword = 0;

      if (ivl_expr_type(rval) != IVL_EX_SIGNAL)
	    return;

      switch (ivl_statement_type(net)) {
	  case IVL_ST_CASSIGN:
	    command_name = "%cassign";
	    break;
	  case IVL_ST_FORCE:
	    command_name = "%force";
	    break;
	  default:
	    command_name = "ERROR";
	    assert(0);
	    break;
      }

      rsig = ivl_expr_signal(rval);
      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);
      lsig = ivl_lval_sig(lval);

	/* We do not currently support driving a signal to a bit or
	 * part select (this could give us multiple drivers). */
      part_off_ex = ivl_lval_part_off(lval);
	/* This should be verified in force_vector_to_lval() which is called
	 * before this procedure. */
      if (part_off_ex) {
	    assert(number_is_immediate(part_off_ex, IMM_WID, 0));
	    assert(! number_is_unknown(part_off_ex));
      }
      if (ivl_signal_width(lsig) > ivl_signal_width(rsig) ||
          (part_off_ex && get_number_immediate(part_off_ex) != 0)) {
	      /* Normalize the bit/part select. */
	    long real_msb = ivl_signal_msb(lsig);
	    long real_lsb = ivl_signal_lsb(lsig);
	    long use_wid = ivl_signal_width(rsig);
	    long use_lsb, use_msb;
	    if (real_msb >= real_lsb) {
		  use_lsb = get_number_immediate(part_off_ex);
		  use_lsb += real_lsb;
		  use_msb = use_lsb + use_wid - 1;
	    } else {
		  use_lsb = real_lsb;
		  use_lsb -= get_number_immediate(part_off_ex);
		  use_msb = use_lsb;
		  use_msb -= use_wid - 1;
	    }
	      /* This is technically the wrong file/line object. It should
	       * be the statement. */
	    fprintf(stderr, "%s:%u: tgt-vvp sorry: cannot %s signal to a ",
	            ivl_expr_file(rval), ivl_expr_lineno(rval), command_name);
	    if (use_wid > 1) {
		  fprintf(stderr, "part select (%s[%lu:%lu]).\n",
		          ivl_signal_basename(lsig), use_msb, use_lsb);
	    } else {
		  fprintf(stderr, "bit select (%s[%lu]).\n",
		          ivl_signal_basename(lsig), use_lsb);
	    }
	    vvp_errors += 1;
      }

	/* At least for now, only handle force to fixed words of an array. */
      if ((lword_idx = ivl_lval_idx(lval)) != 0) {
	    assert(number_is_immediate(lword_idx, IMM_WID, 0));
	    assert(! number_is_unknown(lword_idx));
	    use_lword = get_number_immediate(lword_idx);
	      /* We do not currently support using a word from a variable
	       * array as the L-value (Icarus extension). */
	    if (ivl_signal_type(lsig) == IVL_SIT_REG) {
		    /* Normalize the array access. */
		  long real_word = use_lword;
		  real_word += ivl_signal_array_base(lsig);
		    /* This is technically the wrong file/line object. It
		     * should be the statement. */
		  fprintf(stderr, "%s:%u: tgt-vvp sorry: cannot %s to the "
		          "word of a variable array (%s[%ld]).\n",
		          ivl_expr_file(rval), ivl_expr_lineno(rval),
		          command_name, ivl_signal_basename(lsig), real_word);
		  vvp_errors += 1;
	    }
      }

      if ((rword_idx = ivl_expr_oper1(rval)) != 0) {
	    assert(ivl_signal_dimensions(rsig) != 0);
	    assert(number_is_immediate(rword_idx, IMM_WID, 0));
	    assert(! number_is_unknown(rword_idx));
	    use_rword = get_number_immediate(rword_idx);
	      /* We do not currently support using a word from a variable
	       * array as the R-value. */
	    if (ivl_signal_type(rsig) == IVL_SIT_REG) {
		    /* Normalize the array access. */
		  long real_word = use_rword;
		  real_word += ivl_signal_array_base(rsig);
		  fprintf(stderr, "%s:%u: tgt-vvp sorry: cannot %s from the "
		          "word of a variable array (%s[%ld]).\n",
		          ivl_expr_file(rval), ivl_expr_lineno(rval),
		          command_name, ivl_signal_basename(rsig), real_word);
		  vvp_errors += 1;
	    }
      } else {
	    assert(ivl_signal_dimensions(rsig) == 0);
	    use_rword = 0;
      }

      fprintf(vvp_out, "    %s/link", command_name);
      fprintf(vvp_out, " v%p_%lu", lsig, use_lword);
      fprintf(vvp_out, ", v%p_%lu;\n", rsig, use_rword);
}

static int show_stmt_cassign(ivl_statement_t net)
{
      ivl_expr_t rval;
      ivl_signal_t sig;

      rval = ivl_stmt_rval(net);
      assert(rval);

      sig = ivl_lval_sig(ivl_stmt_lval(net, 0));
      if (sig && ivl_signal_data_type(sig) == IVL_VT_REAL) {
	    int res;

	    res = draw_eval_real(ivl_stmt_rval(net));
	    clr_word(res);

	    force_real_to_lval(net, res);
      } else {
	    struct vector_info rvec;

	    rvec = draw_eval_expr(rval, STUFF_OK_47);

	      /* Write out initial continuous assign instructions to assign
	         the expression value to the l-value. */
	    force_vector_to_lval(net, rvec);
      }

      force_link_rval(net, rval);

      return 0;
}

/*
 * Handle the deassign similar to cassign. The lvals must all be
 * vectors without bit or part selects. Simply call %deassign for all
 * the values.
 */
static int show_stmt_deassign(ivl_statement_t net)
{
      ivl_signal_t sig = ivl_lval_sig(ivl_stmt_lval(net, 0));
      unsigned lidx;

      if (sig && ivl_signal_data_type(sig) == IVL_VT_REAL) {
	    ivl_lval_t lval;

	    assert(ivl_stmt_lvals(net) == 1);
	    lval = ivl_stmt_lval(net, 0);
	    assert(ivl_lval_width(lval) == 1);
	    assert(ivl_lval_part_off(lval) == 0);
	    assert(ivl_lval_idx(lval) == 0);

	    fprintf(vvp_out, "    %%deassign/wr v%p_0;\n", sig);
	    return 0;
      }

      for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
	    ivl_lval_t lval = ivl_stmt_lval(net, lidx);
	    ivl_signal_t lsig = ivl_lval_sig(lval);

	    ivl_expr_t word_idx = ivl_lval_idx(lval);
	    unsigned long use_word = 0;
	    unsigned use_wid;
	    ivl_expr_t part_off_ex;
	    unsigned part_off;

	    assert(lsig != 0);
	    assert(ivl_lval_mux(lval) == 0);

	    use_wid = ivl_lval_width(lval);
	    part_off_ex = ivl_lval_part_off(lval);
	    part_off = 0;
	    if (part_off_ex != 0) {
		  assert(number_is_immediate(part_off_ex, 64, 0));
		  assert(! number_is_unknown(part_off_ex));
		  part_off = get_number_immediate(part_off_ex);
	    }

	    if (word_idx != 0) {
		  assert(number_is_immediate(word_idx, IMM_WID, 0));
		  assert(! number_is_unknown(word_idx));
		  use_word = get_number_immediate(word_idx);
	    }


	    fprintf(vvp_out, "    %%deassign v%p_%lu, %u, %u;\n",
		    lsig, use_word, part_off, use_wid);
      }

      return 0;
}

static int show_stmt_condit(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      unsigned lab_false, lab_out;
      ivl_expr_t expr = ivl_stmt_cond_expr(net);
      struct vector_info cond
	    = draw_eval_expr(expr, STUFF_OK_XZ|STUFF_OK_47|STUFF_OK_RO);

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
 * The delay statement comes from Verilog code like this:
 *
 *        ...
 *        #<delay> <stmt>;
 */
static int show_stmt_delay(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      uint64_t delay = ivl_stmt_delay_val(net);
      ivl_statement_t stmt = ivl_stmt_sub_stmt(net);

      unsigned long low = delay % UINT64_C(0x100000000);
      unsigned long hig = delay / UINT64_C(0x100000000);

      fprintf(vvp_out, "    %%delay %lu, %lu;\n", low, hig);
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
      ivl_expr_t expr = ivl_stmt_delay_expr(net);
      ivl_statement_t stmt = ivl_stmt_sub_stmt(net);

      switch (ivl_expr_value(expr)) {

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC: {
		struct vector_info del = draw_eval_expr(expr, 0);
		fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n",
			del.base, del.wid);
		clr_vector(del);
		break;
	  }

	  case IVL_VT_REAL: {
		int word = draw_eval_real(expr);
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
      fprintf(vvp_out, "    %%disable S_%p;\n", target);

      return rc;
}

static int show_stmt_force(ivl_statement_t net)
{
      ivl_expr_t rval;
      ivl_signal_t sig;

      rval = ivl_stmt_rval(net);
      assert(rval);

      sig = ivl_lval_sig(ivl_stmt_lval(net, 0));
      if (sig && ivl_signal_data_type(sig) == IVL_VT_REAL) {
            int res;

            res = draw_eval_real(ivl_stmt_rval(net));
            clr_word(res);

            force_real_to_lval(net, res);
      } else {
            struct vector_info rvec;

            rvec = draw_eval_expr(rval, STUFF_OK_47);

              /* Write out initial continuous assign instructions to assign
                 the expression value to the l-value. */
            force_vector_to_lval(net, rvec);
      }

      force_link_rval(net, rval);

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
      ivl_scope_t scope = ivl_stmt_block_scope(net);
      unsigned is_named = (scope != 0);

      unsigned out = transient_id++;
      unsigned id_base = transient_id;

	/* cnt is the number of sub-threads. If the fork-join has no
	   name, then we can put one of the sub-threads in the current
	   thread, so decrement the count by one and use the current
	   scope for all the threads. */
      if (! is_named) {
	    cnt -= 1;
	    scope = sscope;
      }

      transient_id += cnt;

	/* Draw a fork statement for all but one of the threads of the
	   fork/join. Send the threads off to a bit of code where they
	   are implemented. */
      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    fprintf(vvp_out, "    %%fork t_%u, S_%p;\n",
		    id_base+idx, scope);
      }

	/* If we are putting one sub-thread into the current thread,
	   then draw its code here. */
      if (! is_named)
	    rc += show_statement(ivl_stmt_block_stmt(net, cnt), scope);


	/* Generate enough joins to collect all the sub-threads. */
      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    fprintf(vvp_out, "    %%join;\n");
      }
      fprintf(vvp_out, "    %%jmp t_%u;\n", out);

	/* Change the compiling scope to be the named forks scope. */
      if (is_named) fprintf(vvp_out, "    .scope S_%p;\n", scope);
	/* Generate the sub-threads themselves. */
      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    fprintf(vvp_out, "t_%u ;\n", id_base+idx);
	    clear_expression_lookaside();
	    rc += show_statement(ivl_stmt_block_stmt(net, idx), scope);
	    fprintf(vvp_out, "    %%end;\n");
      }
	/* Return to the previous scope. */
      if (is_named) fprintf(vvp_out, "    .scope S_%p;\n", sscope);

	/* This is the label for the out. Use this to branch around
	   the implementations of all the child threads. */
      clear_expression_lookaside();
      fprintf(vvp_out, "t_%u ;\n", out);

      return rc;
}

static int show_stmt_free(ivl_statement_t net)
{
      ivl_scope_t scope = ivl_stmt_call(net);

      fprintf(vvp_out, "    %%free S_%p;\n", scope);
      return 0;
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
      ivl_signal_t sig = ivl_lval_sig(ivl_stmt_lval(net, 0));
      unsigned lidx;

      if (sig && ivl_signal_data_type(sig) == IVL_VT_REAL) {
	    unsigned type = 0;
	    ivl_lval_t lval;

	    assert(ivl_stmt_lvals(net) == 1);
	    lval = ivl_stmt_lval(net, 0);
	    assert(ivl_lval_width(lval) == 1);
	    assert(ivl_lval_part_off(lval) == 0);
	    assert(ivl_lval_idx(lval) == 0);

	    if (ivl_signal_type(sig) == IVL_SIT_REG) type = 1;

	    fprintf(vvp_out, "    %%release/wr v%p_0, %u;\n", sig, type);
	    return 0;
      }

      for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
	    ivl_lval_t lval = ivl_stmt_lval(net, lidx);
	    ivl_signal_t lsig = ivl_lval_sig(lval);
	    const char*opcode = 0;

	    ivl_expr_t word_idx = ivl_lval_idx(lval);
	    unsigned long use_word = 0;
	    unsigned use_wid;
	    ivl_expr_t part_off_ex;
	    unsigned part_off;

	    assert(lsig != 0);
	    assert(ivl_lval_mux(lval) == 0);

	    use_wid = ivl_lval_width(lval);
	    part_off_ex = ivl_lval_part_off(lval);
	    part_off = 0;
	    if (part_off_ex != 0) {
		  assert(number_is_immediate(part_off_ex, 64, 0));
		  assert(! number_is_unknown(part_off_ex));
		  part_off = get_number_immediate(part_off_ex);
	    }

	    switch (ivl_signal_type(lsig)) {
		case IVL_SIT_REG:
		  opcode = "reg";
		  break;
		default:
		  opcode = "net";
		  break;
	    }

	    if (word_idx != 0) {
		  assert(number_is_immediate(word_idx, IMM_WID, 0));
		  assert(! number_is_unknown(word_idx));
		  use_word = get_number_immediate(word_idx);
	    }

	      /* Generate the appropriate release statement for this
		 l-value. */
	    fprintf(vvp_out, "    %%release/%s v%p_%lu, %u, %u;\n",
		    opcode, lsig, use_word, part_off, use_wid);
      }

      return 0;
}

static int show_stmt_repeat(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      unsigned lab_top = local_count++, lab_out = local_count++;
      ivl_expr_t expr = ivl_stmt_cond_expr(net);
      struct vector_info cnt = draw_eval_expr(expr, 0);
      const char *sign = ivl_expr_signed(expr) ? "s" : "u";

	/* Test that 0 < expr */
      fprintf(vvp_out, "T_%u.%u %%cmp/%s 0, %u, %u;\n", thread_count,
	      lab_top, sign, cnt.base, cnt.wid);
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

/*
 * The trigger statement is straight forward. All we have to do is
 * write a single bit of fake data to the event object.
 */
static int show_stmt_trigger(ivl_statement_t net)
{
      ivl_event_t ev = ivl_stmt_events(net, 0);
      assert(ev);
      fprintf(vvp_out, "    %%set/v E_%p, 0,1;\n", ev);
      return 0;
}

static int show_stmt_utask(ivl_statement_t net)
{
      ivl_scope_t task = ivl_stmt_call(net);

      fprintf(vvp_out, "    %%fork TD_%s",
	      vvp_mangle_id(ivl_scope_name(task)));
      fprintf(vvp_out, ", S_%p;\n", task);
      fprintf(vvp_out, "    %%join;\n");
      clear_expression_lookaside();
      return 0;
}

static int show_stmt_wait(ivl_statement_t net, ivl_scope_t sscope)
{
      if (ivl_stmt_nevent(net) == 1) {
	    ivl_event_t ev = ivl_stmt_events(net, 0);
	    fprintf(vvp_out, "    %%wait E_%p;\n", ev);

      } else {
	    unsigned idx;
	    static unsigned int cascade_counter = 0;
	    ivl_event_t ev = ivl_stmt_events(net, 0);
	    fprintf(vvp_out, "Ewait_%u .event/or E_%p", cascade_counter, ev);

	    for (idx = 1 ;  idx < ivl_stmt_nevent(net) ;  idx += 1) {
		  ev = ivl_stmt_events(net, idx);
		  fprintf(vvp_out, ", E_%p", ev);
	    }
	    fprintf(vvp_out, ";\n    %%wait Ewait_%u;\n", cascade_counter);
	    cascade_counter += 1;
      }
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
	    assert(result.base);
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
	   test is repeated, and also draw the out label. */
      fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count, top_label);
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, out_label);
      clear_expression_lookaside();
      return rc;
}

static int show_system_task_call(ivl_statement_t net)
{
      unsigned parm_count = ivl_stmt_parm_count(net);

      if (parm_count == 0) {
	    fprintf(vvp_out, "    %%vpi_call %u %u \"%s\";\n",
	            ivl_file_table_index(ivl_stmt_file(net)),
	            ivl_stmt_lineno(net), ivl_stmt_name(net));
	    clear_expression_lookaside();
	    return 0;
      }

      draw_vpi_task_call(net);

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

	  case IVL_ST_ALLOC:
	    rc += show_stmt_alloc(net);
	    break;

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

	  case IVL_ST_CASER:
	    rc += show_stmt_case_r(net, sscope);
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

	  case IVL_ST_FREE:
	    rc += show_stmt_free(net);
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
      unsigned idx;
      ivl_scope_t scope = ivl_process_scope(net);
      ivl_statement_t stmt = ivl_process_stmt(net);

      int push_flag = 0;

      for (idx = 0 ;  idx < ivl_process_attr_cnt(net) ;  idx += 1) {

	    ivl_attribute_t attr = ivl_process_attr_val(net, idx);

	    if (strcmp(attr->key, "_ivl_schedule_push") == 0) {

		  push_flag = 1;

	    } else if (strcmp(attr->key, "ivl_combinational") == 0) {

		  push_flag = 1;

	    }
      }

      local_count = 0;
      fprintf(vvp_out, "    .scope S_%p;\n", scope);

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

      if (push_flag) {
	    fprintf(vvp_out, "    .thread T_%d, $push;\n", thread_count);
      } else {
	    fprintf(vvp_out, "    .thread T_%d;\n", thread_count);
      }

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
