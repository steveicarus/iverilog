/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "vvp_priv.h"
# include  <string.h>
# include  <assert.h>
# include  <stdlib.h>

#ifdef __MINGW32__ /* MinGW has inconsistent %p output. */
#define snprintf _snprintf
#endif


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

enum slice_type_e {
      SLICE_NO_TYPE = 0,
      SLICE_SIMPLE_VECTOR,
      SLICE_PART_SELECT_STATIC,
      SLICE_PART_SELECT_DYNAMIC,
      SLICE_MEMORY_WORD_STATIC,
      SLICE_MEMORY_WORD_DYNAMIC
};

struct vec_slice_info {
      enum slice_type_e type;

      union {
	    struct {
		  unsigned long use_word;
	    } simple_vector;

	    struct {
		  unsigned long part_off;
	    } part_select_static;

	    struct {
		    /* Index reg that holds the memory word index */
		  int word_idx_reg;
		    /* Stored x/non-x flag */
		  unsigned x_flag;
	    } part_select_dynamic;

	    struct {
		  unsigned long use_word;
	    } memory_word_static;

	    struct {
		    /* Index reg that holds the memory word index */
		  int word_idx_reg;
		    /* Stored x/non-x flag */
		  unsigned x_flag;
	    } memory_word_dynamic;
      } u_;
};

static void get_vec_from_lval_slice(ivl_lval_t lval, struct vec_slice_info*slice,
				    unsigned bit, unsigned wid)
{
      ivl_signal_t sig = ivl_lval_sig(lval);
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

      if (ivl_signal_dimensions(sig)==0 && part_off_ex==0 && word_ix==0
	  && part_off==0 && wid==ivl_signal_width(sig)) {

	    slice->type = SLICE_SIMPLE_VECTOR;
	    slice->u_.simple_vector.use_word = use_word;
	    fprintf(vvp_out, "    %%load/v %u, v%p_%lu, %u;\n",
		    bit, sig, use_word, wid);

      } else if (ivl_signal_dimensions(sig)==0 && part_off_ex==0 && word_ix==0) {

	    assert(use_word == 0);

	    slice->type = SLICE_PART_SELECT_STATIC;
	    slice->u_.part_select_static.part_off = part_off;

	    fprintf(vvp_out, "    %%ix/load 1, %lu, 0;\n", part_off);
	    fprintf(vvp_out, "    %%load/x1p %u, v%p_0, %u;\n", bit, sig, wid);

      } else if (ivl_signal_dimensions(sig)==0 && part_off_ex!=0 && word_ix==0) {

	    unsigned skip_set = transient_id++;
	    unsigned out_set  = transient_id++;

	    assert(use_word == 0);
	    assert(part_off == 0);

	    slice->type = SLICE_PART_SELECT_DYNAMIC;

	    draw_eval_expr_into_integer(part_off_ex, 1);

	    slice->u_.part_select_dynamic.word_idx_reg = allocate_word();
	    slice->u_.part_select_dynamic.x_flag = allocate_vector(1);

	    fprintf(vvp_out, "    %%mov %u, %u, 1;\n",
		    slice->u_.part_select_dynamic.x_flag, 4);
	    fprintf(vvp_out, "    %%mov/wu %d, %d;\n",
		    slice->u_.part_select_dynamic.word_idx_reg, 1);

	    fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
	    fprintf(vvp_out, "    %%load/x1p %u, v%p_0, %u;\n", bit, sig, wid);
	    fprintf(vvp_out, "    %%jmp t_%u;\n", out_set);
	    fprintf(vvp_out, "t_%u ;\n", skip_set);
	    fprintf(vvp_out, "    %%mov %u, 2, %u;\n", bit, wid);
	    fprintf(vvp_out, "t_%u ;\n", out_set);

      } else if (ivl_signal_dimensions(sig) > 0 && word_ix == 0) {

	    slice->type = SLICE_MEMORY_WORD_STATIC;
	    slice->u_.memory_word_static.use_word = use_word;
	    if (use_word < ivl_signal_array_count(sig)) {
		  fprintf(vvp_out, "    %%ix/load 3, %lu, 0;\n",
			  use_word);
		  fprintf(vvp_out, "    %%load/av %u, v%p, %u;\n",
			  bit, sig, wid);
	    } else {
		  fprintf(vvp_out, "    %%mov %u, 2, %u; OUT OF BOUNDS\n",
			  bit, wid);
	    }

      } else if (ivl_signal_dimensions(sig) > 0 && word_ix != 0) {

	    unsigned skip_set = transient_id++;
	    unsigned out_set  = transient_id++;
	    slice->type = SLICE_MEMORY_WORD_DYNAMIC;

	    draw_eval_expr_into_integer(word_ix, 3);
	    slice->u_.memory_word_dynamic.word_idx_reg = allocate_word();
	    slice->u_.memory_word_dynamic.x_flag = allocate_vector(1);
	    fprintf(vvp_out, "    %%mov/wu %d, 3;\n",
		    slice->u_.memory_word_dynamic.word_idx_reg);
	    fprintf(vvp_out, "    %%mov %u, 4, 1;\n",
		    slice->u_.memory_word_dynamic.x_flag);

	    fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
	    fprintf(vvp_out, "    %%ix/load 1, 0, 0;\n");
	    fprintf(vvp_out, "    %%load/av %u, v%p, %u;\n",
		    bit, sig, wid);
	    fprintf(vvp_out, "    %%jmp t_%u;\n", out_set);
	    fprintf(vvp_out, "t_%u ;\n", skip_set);
	    fprintf(vvp_out, "    %%mov %u, 2, %u;\n", bit, wid);
	    fprintf(vvp_out, "t_%u ;\n", out_set);

      } else {
	    assert(0);
      }
}

static struct vector_info get_vec_from_lval(ivl_statement_t net,
					    struct vec_slice_info*slices)
{
      struct vector_info res;
      unsigned lidx;
      unsigned cur_bit;

      res.wid = ivl_stmt_lwidth(net);
      res.base = allocate_vector(res.wid);

      cur_bit = 0;
      for (lidx = 0 ; lidx < ivl_stmt_lvals(net) ; lidx += 1) {
	    unsigned bidx;
	    ivl_lval_t lval;
	    unsigned bit_limit = res.wid - cur_bit;

	    lval = ivl_stmt_lval(net, lidx);

	    if (bit_limit > ivl_lval_width(lval))
		  bit_limit = ivl_lval_width(lval);

	    bidx = res.base + cur_bit;

	    get_vec_from_lval_slice(lval, slices+lidx, bidx, bit_limit);

	    cur_bit += bit_limit;
      }

      return res;
}

static void put_vec_to_lval_slice(ivl_lval_t lval, struct vec_slice_info*slice,
				  unsigned bit, unsigned wid)
{
      unsigned skip_set = transient_id++;
      struct vector_info tmp;
      ivl_signal_t sig = ivl_lval_sig(lval);

	/* If the slice of the l-value is a BOOL variable, then cast
	   the data to a BOOL vector so that the stores can be valid. */
      if (ivl_signal_data_type(sig) == IVL_VT_BOOL) {
	    fprintf(vvp_out, "    %%cast2 %u, %u, %u;\n", bit, bit, wid);
      }

      switch (slice->type) {
	  default:
	    assert(0);
	    break;

	  case SLICE_SIMPLE_VECTOR:
	    fprintf(vvp_out, "    %%set/v v%p_%lu, %u, %u;\n",
		    sig, slice->u_.simple_vector.use_word, bit, wid);
	    break;

	  case SLICE_PART_SELECT_STATIC:
	    fprintf(vvp_out, "    %%ix/load 0, %lu, 0;\n",
		    slice->u_.part_select_static.part_off);
	    fprintf(vvp_out, "    %%set/x0 v%p_0, %u, %u;\n", sig, bit, wid);
	    break;

	  case SLICE_PART_SELECT_DYNAMIC:
	    fprintf(vvp_out, "    %%jmp/1 t_%u, %u;\n", skip_set,
		    slice->u_.part_select_dynamic.x_flag);
	    fprintf(vvp_out, "    %%mov/wu 0, %d;\n",
		    slice->u_.part_select_dynamic.word_idx_reg);
	    fprintf(vvp_out, "    %%set/x0 v%p_0, %u, %u;\n", sig, bit, wid);
	    fprintf(vvp_out, "t_%u ;\n", skip_set);
	    break;

	  case SLICE_MEMORY_WORD_STATIC:
	    if (slice->u_.simple_vector.use_word >= ivl_signal_array_count(sig))
		  break;
	    fprintf(vvp_out, "    %%ix/load 3, %lu, 0;\n",
		    slice->u_.simple_vector.use_word);
	    fprintf(vvp_out, "    %%set/av v%p, %u, %u;\n",
		    sig, bit, wid);
	    break;

	  case SLICE_MEMORY_WORD_DYNAMIC:
	    fprintf(vvp_out, "    %%jmp/1 t_%u, %u;\n", skip_set,
		    slice->u_.memory_word_dynamic.x_flag);
	    fprintf(vvp_out, "    %%mov/wu 3, %d;\n",
		    slice->u_.memory_word_dynamic.word_idx_reg);
	    fprintf(vvp_out, "    %%set/av v%p, %u, %u;\n",
		    ivl_lval_sig(lval), bit, wid);
	    fprintf(vvp_out, "t_%u ;\n", skip_set);

	    tmp.base = slice->u_.memory_word_dynamic.x_flag;
	    tmp.wid = 1;
	    clr_vector(tmp);
	    clr_word(slice->u_.memory_word_dynamic.word_idx_reg);
	    break;
      }
}

static void put_vec_to_lval(ivl_statement_t net, struct vec_slice_info*slices,
			    struct vector_info res)
{
      unsigned lidx;
      unsigned cur_bit;

      cur_bit = 0;
      for (lidx = 0 ; lidx < ivl_stmt_lvals(net) ; lidx += 1) {
	    unsigned bidx;
	    ivl_lval_t lval;
	    unsigned bit_limit = res.wid - cur_bit;

	    lval = ivl_stmt_lval(net, lidx);

	    if (bit_limit > ivl_lval_width(lval))
		  bit_limit = ivl_lval_width(lval);

	    bidx = res.base + cur_bit;

	    put_vec_to_lval_slice(lval, slices+lidx, bidx, bit_limit);

	    cur_bit += bit_limit;
      }
}

static ivl_type_t draw_lval_expr(ivl_lval_t lval)
{
      ivl_lval_t lval_nest = ivl_lval_nest(lval);
      ivl_signal_t lval_sig = ivl_lval_sig(lval);
      ivl_type_t sub_type;

      if (lval_nest) {
	    sub_type = draw_lval_expr(lval_nest);
      } else {
	    assert(lval_sig);
	    sub_type = ivl_signal_net_type(lval_sig);
	    assert(ivl_type_base(sub_type) == IVL_VT_CLASS);
	    fprintf(vvp_out, "    %%load/obj v%p_0;\n", lval_sig);
      }

      assert(ivl_type_base(sub_type) == IVL_VT_CLASS);
      fprintf(vvp_out, "    %%prop/obj %d;\n", ivl_lval_property_idx(lval));
      fprintf(vvp_out, "    %%pop/obj 1, 1;\n");
      return ivl_type_prop_type(sub_type, ivl_lval_property_idx(lval));
}

#if 0
static void set_vec_to_lval_slice_nest(ivl_lval_t lval, unsigned bit, unsigned wid)
{
      ivl_lval_t lval_nest = ivl_lval_nest(lval);
      ivl_type_t ltype = draw_lval_expr(lval_nest);
      assert(ivl_type_base(ltype) == IVL_VT_CLASS);

      fprintf(vvp_out, "    %%store/prop/v %d, %u, %u;\n",
	      ivl_lval_property_idx(lval), bit, wid);
      fprintf(vvp_out, "    %%pop/obj 1, 0;\n");
}
#endif

#if 0
static void set_vec_to_lval_slice(ivl_lval_t lval, unsigned bit, unsigned wid)
{
      ivl_signal_t sig  = ivl_lval_sig(lval);
      ivl_expr_t part_off_ex = ivl_lval_part_off(lval);
      unsigned long part_off = 0;

	/* Although Verilog doesn't support it, we'll handle
	   here the case of an l-value part select of an array
	   word if the address is constant. */
      ivl_expr_t word_ix = ivl_lval_idx(lval);
      unsigned long use_word = 0;

	/* If the l-value is nested, then it is something like a class
	   with a chain of member names, so handle that elsewhere. */
      if (ivl_lval_nest(lval)) {
	    set_vec_to_lval_slice_nest(lval, bit, wid);
	    return;
      }

      if (part_off_ex == 0) {
	    part_off = 0;
      } else if (number_is_immediate(part_off_ex, IMM_WID, 0) &&
                 !number_is_unknown(part_off_ex)) {
	    part_off = get_number_immediate(part_off_ex);
	    part_off_ex = 0;
      }

	/* If the word index is a constant expression, then evaluate
	   it to select the word, and pay no further heed to the
	   expression itself. Out-of-bounds and undefined indices are
	   converted to a canonical index of 'bx during elaboration,
	   and we don't try to optimise that case. */
      if (word_ix && number_is_immediate(word_ix, IMM_WID, 0) &&
          !number_is_unknown(word_ix)) {
	    use_word = get_number_immediate(word_ix);
	    assert(use_word < ivl_signal_array_count(sig));
	    word_ix = 0;
      }

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
		  int part_off_reg = allocate_word();
		  draw_eval_expr_into_integer(part_off_ex, part_off_reg);
		  fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
		  draw_eval_expr_into_integer(word_ix, 3);
		  fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
		  fprintf(vvp_out, "    %%ix/mov 1, %u;\n", part_off_reg);
		  clr_word(part_off_reg);
	    } else {
		  draw_eval_expr_into_integer(part_off_ex, 1);
		  fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
		  fprintf(vvp_out, "    %%ix/load 3, %lu, 0;\n", use_word);
	    }
	    fprintf(vvp_out, "    %%set/av v%p, %u, %u;\n",
		    sig, bit, wid);
	    fprintf(vvp_out, "t_%u ;\n", skip_set);

      } else if ((part_off>0 || ivl_lval_width(lval)!=ivl_signal_width(sig))
		 && ivl_signal_dimensions(sig) > 0) {

	      /* Here we have a part select write into an array word. */
	    unsigned skip_set = transient_id++;
	    if (word_ix) {
		  draw_eval_expr_into_integer(word_ix, 3);
		  fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
	    } else {
		  fprintf(vvp_out, "    %%ix/load 3, %lu, 0;\n", use_word);
	    }
	    fprintf(vvp_out, "    %%ix/load 1, %lu, 0;\n", part_off);
	    fprintf(vvp_out, "    %%set/av v%p, %u, %u;\n",
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
		  fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
		  fprintf(vvp_out, "    %%ix/load 1, %lu, 0;\n", part_off);
		  fprintf(vvp_out, "    %%set/av v%p, %u, %u;\n",
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
		  fprintf(vvp_out, "    %%ix/load 1, 0, 0;\n");
		  fprintf(vvp_out, "    %%ix/load 3, %lu, 0;\n", use_word);
		  fprintf(vvp_out, "    %%set/av v%p, %u, %u;\n",
			  sig, bit, wid);

	    } else {
		  unsigned skip_set = transient_id++;
		  unsigned index_reg = 3;
		  draw_eval_expr_into_integer(word_ix, index_reg);
		  fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
		  fprintf(vvp_out, "    %%ix/load 1, 0, 0;\n");
		  fprintf(vvp_out, "    %%set/av v%p, %u, %u;\n",
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
#endif
#if 0
/*
 * This is a private function to generate %set code for the
 * statement. At this point, the r-value is evaluated and stored in
 * the res vector, I just need to generate the %set statements for the
 * l-values of the assignment.
 */
static void set_vec_to_lval(ivl_statement_t net, struct vector_info res)
{
      unsigned wid = res.wid;
      unsigned lidx;
      unsigned cur_rbit = 0;

      for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
	    unsigned bidx;
	    unsigned bit_limit = wid - cur_rbit;

	    ivl_lval_t lval = ivl_stmt_lval(net, lidx);

	      /* Reduce bit_limit to the width of this l-value. */
	    if (bit_limit > ivl_lval_width(lval))
		  bit_limit = ivl_lval_width(lval);

	      /* This is the address within the larger r-value of the
		 bit that this l-value takes. */
	    bidx = res.base < 4? res.base : (res.base+cur_rbit);

	    set_vec_to_lval_slice(lval, bidx, bit_limit);

	      /* Now we've consumed this many r-value bits for the
		 current l-value. */
	    cur_rbit += bit_limit;
      }
}
#endif

/*
 * Store a vector from the vec4 stack to the statement l-values. This
 * all assumes that the value to be assigned is already on the top of
 * the stack.
 *
 * NOTE TO SELF: The %store/vec4 takes a width, but the %assign/vec4
 * instructions do not, instead relying on the expression width. I
 * think that it the proper way to do it, so soon I should change the
 * %store/vec4 to not include the width operand.
 */
static void store_vec4_to_lval(ivl_statement_t net)
{
      for (unsigned lidx = 0 ; lidx < ivl_stmt_lvals(net) ; lidx += 1) {
	    ivl_lval_t lval = ivl_stmt_lval(net,lidx);
	    ivl_signal_t lsig = ivl_lval_sig(lval);
	    unsigned lwid = ivl_lval_width(lval);

	    ivl_expr_t part_off_ex = ivl_lval_part_off(lval);
	      /* This is non-nil if the l-val is the word of a memory,
		 and nil otherwise. */
	    ivl_expr_t word_ex = ivl_lval_idx(lval);

	    if (lidx+1 < ivl_stmt_lvals(net))
		  fprintf(vvp_out, "    %%split/vec4 %u;\n", lwid);

	    if (word_ex) {
		    /* Handle index into an array */
		  int word_index = allocate_word();
		  int part_index = 0;
		    /* Calculate the word address into word_index */
		  draw_eval_expr_into_integer(word_ex, word_index);
		    /* If there is a part_offset, calcualte it into part_index. */
		  if (part_off_ex) {
			part_index = allocate_word();
			draw_eval_expr_into_integer(part_off_ex, part_index);
		  }

		  fprintf(vvp_out, "    %%store/vec4a v%p, %d, %d;\n",
			  lsig, word_index, part_index);

		  clr_word(word_index);
		  if (part_index)
			clr_word(part_index);

	    } else if (part_off_ex) {
		    /* Dynamically calculated part offset */
		  int offset_index = allocate_word();
		  draw_eval_expr_into_integer(part_off_ex, offset_index);
		  fprintf(vvp_out, "    %%store/vec4 v%p_0, %d, %u;\n",
			  lsig, offset_index, lwid);
		  clr_word(offset_index);

	    } else {
		    /* No offset expression, so use simpler store function. */
		  assert(lwid == ivl_signal_width(lsig));
		  fprintf(vvp_out, "    %%store/vec4 v%p_0, 0, %u;\n", lsig, lwid);
	    }
      }
}

static int show_stmt_assign_vector(ivl_statement_t net)
{
      ivl_expr_t rval = ivl_stmt_rval(net);
	//struct vector_info res;
      struct vector_info lres = {0, 0};
      struct vec_slice_info*slices = 0;

	/* If this is a compressed assignment, then get the contents
	   of the l-value. We need these values as part of the r-value
	   calculation. */
      if (ivl_stmt_opcode(net) != 0) {
            slices = calloc(ivl_stmt_lvals(net), sizeof(struct vec_slice_info));
	    lres = get_vec_from_lval(net, slices);
      }

	/* Handle the special case that the expression is a real
	   value. Evaluate the real expression, then convert the
	   result to a vector. Then store that vector into the
	   l-value. */
      if (ivl_expr_value(rval) == IVL_VT_REAL) {
            draw_eval_real(rval);
	      /* This is the accumulated with of the l-value of the
		 assignment. */
	    unsigned wid = ivl_stmt_lwidth(net);

	      /* Convert a calculated real value to a vec4 value of
		 the given width. We need to include the width of the
		 result because real values to not have any inherit
		 width. The real value will be popped, and a vec4
		 value pushed. */
	    fprintf(vvp_out, "    %%cvt/vr %u;\n", wid);

      } else {
	    unsigned wid = ivl_stmt_lwidth(net);
	    draw_eval_vec4(rval, 0);
	    if (ivl_expr_width(rval)==wid) {
		  ; /* Normally, the rval expression size is correct. */
	    } else if (ivl_expr_signed(rval)) {
		  fprintf(vvp_out, "    %%pad/s %u;\n", wid);
	    } else {
		  fprintf(vvp_out, "    %%pad/u %u;\n", wid);
	    }
	      //res.base = 0; // XXXX This is just to suppress the clr_vector below.
	      //res.wid = 0;
      }

      switch (ivl_stmt_opcode(net)) {
	  case 0:
	    store_vec4_to_lval(net);
	    break;
#if 0
	      // XXXX These need to be converted to vec4 style.
	  case '+':
	    if (res.base > 3) {
		  fprintf(vvp_out, "    %%add %u, %u, %u;\n",
			  res.base, lres.base, res.wid);
		  clr_vector(lres);
	    } else {
		  fprintf(vvp_out, "    %%add %u, %u, %u;\n",
			  lres.base, res.base, res.wid);
		  res.base = lres.base;
	    }
	    put_vec_to_lval(net, slices, res);
	    break;

	  case '-':
	    fprintf(vvp_out, "    %%sub %u, %u, %u;\n",
		    lres.base, res.base, res.wid);
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
		    res.base, lres.base, res.wid);
	    clr_vector(lres);
	    put_vec_to_lval(net, slices, res);
	    break;

	  case '*':
	    if (res.base > 3) {
		  fprintf(vvp_out, "    %%mul %u, %u, %u;\n",
			  res.base, lres.base, res.wid);
		  clr_vector(lres);
	    } else {
		  fprintf(vvp_out, "    %%mul %u, %u, %u;\n",
			  lres.base, res.base, res.wid);
		  res.base = lres.base;
	    }
	    put_vec_to_lval(net, slices, res);
	    break;

	  case '/':
	    fprintf(vvp_out, "    %%div%s %u, %u, %u;\n",
		    ivl_expr_signed(rval)? "/s" : "",
		    lres.base, res.base, res.wid);
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
		    res.base, lres.base, res.wid);
	    clr_vector(lres);
	    put_vec_to_lval(net, slices, res);
	    break;

	  case '%':
	    fprintf(vvp_out, "    %%mod%s %u, %u, %u;\n",
		    ivl_expr_signed(rval)? "/s" : "",
		    lres.base, res.base, res.wid);
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
		    res.base, lres.base, res.wid);
	    clr_vector(lres);
	    put_vec_to_lval(net, slices, res);
	    break;

	  case '&':
	    if (res.base > 3) {
		  fprintf(vvp_out, "    %%and %u, %u, %u;\n",
			  res.base, lres.base, res.wid);
		  clr_vector(lres);
	    } else {
		  fprintf(vvp_out, "    %%and %u, %u, %u;\n",
			  lres.base, res.base, res.wid);
		  res.base = lres.base;
	    }
	    put_vec_to_lval(net, slices, res);
	    break;

	  case '|':
	    if (res.base > 3) {
		  fprintf(vvp_out, "    %%or %u, %u, %u;\n",
			  res.base, lres.base, res.wid);
		  clr_vector(lres);
	    } else {
		  fprintf(vvp_out, "    %%or %u, %u, %u;\n",
			  lres.base, res.base, res.wid);
		  res.base = lres.base;
	    }
	    put_vec_to_lval(net, slices, res);
	    break;

	  case '^':
	    if (res.base > 3) {
		  fprintf(vvp_out, "    %%xor %u, %u, %u;\n",
			  res.base, lres.base, res.wid);
		  clr_vector(lres);
	    } else {
		  fprintf(vvp_out, "    %%xor %u, %u, %u;\n",
			  lres.base, res.base, res.wid);
		  res.base = lres.base;
	    }
	    put_vec_to_lval(net, slices, res);
	    break;

	  case 'l': /* lres <<= res */
	    fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", res.base, res.wid);
	    fprintf(vvp_out, "    %%shiftl/i0 %u, %u;\n", lres.base, res.wid);
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
		    res.base, lres.base, res.wid);
	    break;

	  case 'r': /* lres >>= res */
	    fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", res.base, res.wid);
	    fprintf(vvp_out, "    %%shiftr/i0 %u, %u;\n", lres.base, res.wid);
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
		    res.base, lres.base, res.wid);
	    break;

	  case 'R': /* lres >>>= res */
	    fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", res.base, res.wid);
	    fprintf(vvp_out, "    %%shiftr/s/i0 %u, %u;\n", lres.base, res.wid);
	    fprintf(vvp_out, "    %%mov %u, %u, %u;\n",
		    res.base, lres.base, res.wid);
	    break;
#endif
	  default:
	    fprintf(vvp_out, "; UNSUPPORTED ASSIGNMENT OPCODE: %c\n", ivl_stmt_opcode(net));
	    assert(0);
	    break;
      }

      if (slices)
	    free(slices);

      return 0;
}

/*
 * This function assigns a value to a real variable. This is destined
 * for /dev/null when typed ivl_signal_t takes over all the real
 * variable support.
 */
static int show_stmt_assign_sig_real(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_signal_t var;

      assert(ivl_stmt_opcode(net) == 0);

      draw_eval_real(ivl_stmt_rval(net));

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);
      var = ivl_lval_sig(lval);
      assert(var != 0);

      if (ivl_signal_dimensions(var) == 0) {
	    fprintf(vvp_out, "    %%store/real v%p_0;\n", var);
	    return 0;
      }

	// For now, only support 1-dimensional arrays.
      assert(ivl_signal_dimensions(var) == 1);

      ivl_expr_t word_ex = ivl_lval_idx(lval);
      int word_ix = allocate_word();

	/* If the word index is a constant, then we can write
	   directly to the word and save the index calculation.
	   Out-of-bounds and undefined indices are converted to
	   a canonical index of 'bx during elaboration, and we
	   don't try to optimise that case. */
      if (word_ex && number_is_immediate(word_ex, IMM_WID, 0) &&
	  !number_is_unknown(word_ex)) {
	    unsigned long use_word = get_number_immediate(word_ex);
	    assert(use_word < ivl_signal_array_count(var));
	    fprintf(vvp_out, "    %%ix/load %d, %lu, 0;\n",
		    word_ix, use_word);
	    fprintf(vvp_out, "    %%store/reala v%p, %d;\n",
		    var, word_ix);

      } else {
	    unsigned do_store = transient_id++;
	    unsigned end_store = transient_id++;
	    draw_eval_expr_into_integer(word_ex, word_ix);
	    fprintf(vvp_out, "    %%jmp/0 t_%u, 4;\n", do_store);
	    fprintf(vvp_out, "    %%pop/real 1;\n");
	    fprintf(vvp_out, "    %%jmp t_%u;\n", end_store);
	    fprintf(vvp_out, "t_%u ;\n", do_store);
	    fprintf(vvp_out, "    %%store/reala v%p, %d;\n", var, word_ix);
	    fprintf(vvp_out, "t_%u ;\n", end_store);
      }

      clr_word(word_ix);

      return 0;
}

static int show_stmt_assign_sig_string(ivl_statement_t net)
{
      ivl_lval_t lval = ivl_stmt_lval(net, 0);
      ivl_expr_t rval = ivl_stmt_rval(net);
      ivl_expr_t part = ivl_lval_part_off(lval);
      ivl_expr_t aidx = ivl_lval_idx(lval);
      ivl_signal_t var= ivl_lval_sig(lval);

      assert(ivl_stmt_lvals(net) == 1);
      assert(ivl_stmt_opcode(net) == 0);

	/* Simplest case: no mux. Evaluate the r-value as a string and
	   store the result into the variable. Note that the
	   %store/str opcode pops the string result. */
      if (part == 0 && aidx == 0) {
	    draw_eval_string(rval);
	    fprintf(vvp_out, "    %%store/str v%p_0;\n", var);
	    return 0;
      }

	/* Assign to array. The l-value has an index expression
	   expression so we are assigning to an array word. */
      if (aidx != 0) {
	    unsigned ix;
	    assert(part == 0);
	    draw_eval_string(rval);
	    draw_eval_expr_into_integer(aidx, (ix = allocate_word()));
	    fprintf(vvp_out, "    %%store/stra v%p, %u;\n", var, ix);
	    clr_word(ix);
	    return 0;
      }

	/* Calculate the character select for the word. */
      int mux_word = allocate_word();
      draw_eval_expr_into_integer(part, mux_word);

	/* Evaluate the r-value as a vector. */
      struct vector_info rvec = draw_eval_expr_wid(rval, 8, STUFF_OK_XZ);

      assert(rvec.wid == 8);
      fprintf(vvp_out, "    %%putc/str/v v%p_0, %d, %u;\n", var, mux_word, rvec.base);

      clr_vector(rvec);
      clr_word(mux_word);
      return 0;
}

unsigned width_of_packed_type(ivl_type_t net)
{
      unsigned idx;
      unsigned width = 1;
      for (idx = 0 ; idx < ivl_type_packed_dimensions(net) ; idx += 1) {
	    int lsb = ivl_type_packed_lsb(net,idx);
	    int msb = ivl_type_packed_msb(net,idx);
	    if (lsb <= msb)
		  width *= msb - lsb + 1;
	    else
		  width *= lsb - msb + 1;
      }
      return width;
}

/*
 * This function handles the special case that we assign an array
 * pattern to a dynamic array. Handle this by assigning each
 * element. The array pattern will have a fixed size.
 */
static int show_stmt_assign_darray_pattern(ivl_statement_t net)
{
      int errors = 0;
      ivl_lval_t lval = ivl_stmt_lval(net, 0);
      ivl_expr_t rval = ivl_stmt_rval(net);

      ivl_signal_t var= ivl_lval_sig(lval);
      ivl_type_t var_type= ivl_signal_net_type(var);
      assert(ivl_type_base(var_type) == IVL_VT_DARRAY);

      ivl_type_t element_type = ivl_type_element(var_type);
      unsigned idx;
      struct vector_info rvec;
      unsigned element_width = 1;
      if (ivl_type_base(element_type) == IVL_VT_BOOL)
	    element_width = width_of_packed_type(element_type);
      else if (ivl_type_base(element_type) == IVL_VT_LOGIC)
	    element_width = width_of_packed_type(element_type);

      assert(ivl_expr_type(rval) == IVL_EX_ARRAY_PATTERN);
      for (idx = 0 ; idx < ivl_expr_parms(rval) ; idx += 1) {
	    switch (ivl_type_base(element_type)) {
		case IVL_VT_BOOL:
		case IVL_VT_LOGIC:
		  rvec = draw_eval_expr_wid(ivl_expr_parm(rval,idx),
					    element_width, STUFF_OK_XZ);
		  fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
		  fprintf(vvp_out, "    %%set/dar v%p_0, %u, %u;\n",
			  var, rvec.base, rvec.wid);

		  if (rvec.base >= 4) clr_vector(rvec);
		  break;

		case IVL_VT_REAL:
		  draw_eval_real(ivl_expr_parm(rval,idx));
		  fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
		  fprintf(vvp_out, "    %%store/dar/r v%p_0;\n", var);
		  break;

		case IVL_VT_STRING:
		  draw_eval_string(ivl_expr_parm(rval,idx));
		  fprintf(vvp_out, "    %%ix/load 3, %u, 0;\n", idx);
		  fprintf(vvp_out, "    %%store/dar/str v%p_0;\n", var);
		  break;

		default:
		  fprintf(vvp_out, "; ERROR: show_stmt_assign_darray_pattern: type_base=%d not implemented\n", ivl_type_base(element_type));
		  errors += 1;
		  break;
	    }
      }

      return errors;
}

static int show_stmt_assign_sig_darray(ivl_statement_t net)
{
      int errors = 0;
      ivl_lval_t lval = ivl_stmt_lval(net, 0);
      ivl_expr_t rval = ivl_stmt_rval(net);
      ivl_expr_t part = ivl_lval_part_off(lval);
      ivl_signal_t var= ivl_lval_sig(lval);
      ivl_type_t var_type= ivl_signal_net_type(var);
      assert(ivl_type_base(var_type) == IVL_VT_DARRAY);
      ivl_type_t element_type = ivl_type_element(var_type);

      ivl_expr_t mux  = ivl_lval_idx(lval);

      assert(ivl_stmt_lvals(net) == 1);
      assert(ivl_stmt_opcode(net) == 0);
      assert(part == 0);

      if (mux && (ivl_type_base(element_type)==IVL_VT_REAL)) {
	    draw_eval_real(rval);

	      /* The %set/dar expects the array index to be in index
		 register 3. Calculate the index in place. */
	    draw_eval_expr_into_integer(mux, 3);

	    fprintf(vvp_out, "    %%store/dar/r v%p_0;\n", var);

      } else if (mux && ivl_type_base(element_type)==IVL_VT_STRING) {

	      /* Evaluate the rval into the top of the string stack. */
	    draw_eval_string(rval);

	      /* The %store/dar/s expects the array index to me in index
		 register 3. Calculate the index in place. */
	    draw_eval_expr_into_integer(mux, 3);

	    fprintf(vvp_out, "    %%store/dar/str v%p_0;\n", var);

      } else if (mux) {
	    struct vector_info rvec = draw_eval_expr_wid(rval, ivl_lval_width(lval),
							 STUFF_OK_XZ);
	      /* The %set/dar expects the array index to be in index
		 register 3. Calculate the index in place. */
	    draw_eval_expr_into_integer(mux, 3);

	    fprintf(vvp_out, "    %%set/dar v%p_0, %u, %u;\n",
		    var, rvec.base, rvec.wid);

	    if (rvec.base >= 4) clr_vector(rvec);

      } else if (ivl_expr_type(rval) == IVL_EX_ARRAY_PATTERN) {
	      /* There is no l-value mux, but the r-value is an array
		 pattern. This is a special case of an assignment to
		 elements of the l-value. */
	    errors += show_stmt_assign_darray_pattern(net);

      } else {
	      /* There is no l-value mux, so this must be an
		 assignment to the array as a whole. Evaluate the
		 "object", and store the evaluated result. */
	    errors += draw_eval_object(rval);
	    fprintf(vvp_out, "    %%store/obj v%p_0;\n", var);
      }

      return errors;
}

static int show_stmt_assign_sig_cobject(ivl_statement_t net)
{
      int errors = 0;
      ivl_lval_t lval = ivl_stmt_lval(net, 0);
      ivl_expr_t rval = ivl_stmt_rval(net);
      ivl_signal_t sig= ivl_lval_sig(lval);

      int prop_idx = ivl_lval_property_idx(lval);

      if (prop_idx >= 0) {
	    ivl_type_t sig_type = ivl_signal_net_type(sig);
	    ivl_type_t prop_type = ivl_type_prop_type(sig_type, prop_idx);

	    if (ivl_type_base(prop_type) == IVL_VT_BOOL) {
		  assert(ivl_type_packed_dimensions(prop_type) == 1);
		  assert(ivl_type_packed_msb(prop_type,0) >= ivl_type_packed_lsb(prop_type, 0));
		  int wid = ivl_type_packed_msb(prop_type,0) - ivl_type_packed_lsb(prop_type,0) + 1;

		  struct vector_info val = draw_eval_expr_wid(rval, wid, STUFF_OK_XZ);

		  fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
		  fprintf(vvp_out, "    %%store/prop/v %d, %u, %u; Store in bool property %s\n",
			  prop_idx, val.base, val.wid,
			  ivl_type_prop_name(sig_type, prop_idx));
		  fprintf(vvp_out, "    %%pop/obj 1, 0;\n");
		  clr_vector(val);

	    } else if (ivl_type_base(prop_type) == IVL_VT_LOGIC) {
		  assert(ivl_type_packed_dimensions(prop_type) == 1);
		  assert(ivl_type_packed_msb(prop_type,0) >= ivl_type_packed_lsb(prop_type, 0));
		  int wid = ivl_type_packed_msb(prop_type,0) - ivl_type_packed_lsb(prop_type,0) + 1;

		  struct vector_info val = draw_eval_expr_wid(rval, wid, STUFF_OK_XZ);

		  fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
		  fprintf(vvp_out, "    %%store/prop/v %d, %u, %u; Store in logic property %s\n",
			  prop_idx, val.base, val.wid,
			  ivl_type_prop_name(sig_type, prop_idx));
		  fprintf(vvp_out, "    %%pop/obj 1, 0;\n");
		  clr_vector(val);

	    } else if (ivl_type_base(prop_type) == IVL_VT_REAL) {

		    /* Calculate the real value into the real value
		       stack. The %store/prop/r will pop the stack
		       value. */
		  draw_eval_real(rval);
		  fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
		  fprintf(vvp_out, "    %%store/prop/r %d;\n", prop_idx);
		  fprintf(vvp_out, "    %%pop/obj 1, 0;\n");

	    } else if (ivl_type_base(prop_type) == IVL_VT_STRING) {

		    /* Calculate the string value into the string value
		       stack. The %store/prop/r will pop the stack
		       value. */
		  draw_eval_string(rval);
		  fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
		  fprintf(vvp_out, "    %%store/prop/str %d;\n", prop_idx);
		  fprintf(vvp_out, "    %%pop/obj 1, 0;\n");

	    } else if (ivl_type_base(prop_type) == IVL_VT_DARRAY) {

		    /* The property is a darray, and there is no mux
		       expression to the assignment is of an entire
		       array object. */
		  fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
		  draw_eval_object(rval);
		  fprintf(vvp_out, "    %%store/prop/obj %d;\n", prop_idx);
		  fprintf(vvp_out, "    %%pop/obj 1, 0;\n");

	    } else if (ivl_type_base(prop_type) == IVL_VT_CLASS) {

		    /* The property is a class object. */
		  fprintf(vvp_out, "    %%load/obj v%p_0;\n", sig);
		  draw_eval_object(rval);
		  fprintf(vvp_out, "    %%store/prop/obj %d;\n", prop_idx);
		  fprintf(vvp_out, "    %%pop/obj 1, 0;\n");

	    } else {
		  fprintf(vvp_out, " ; ERROR: ivl_type_base(prop_type) = %d\n",
			  ivl_type_base(prop_type));
		  assert(0);
	    }

      } else {
	      /* There is no property select, so evaluate the r-value
		 as an object and assign the entire object to the
		 variable. */
	    errors += draw_eval_object(rval);
	    fprintf(vvp_out, "    %%store/obj v%p_0;\n", sig);
      }

      return errors;
}

int show_stmt_assign(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_signal_t sig;

      show_stmt_file_line(net, "Blocking assignment.");

      lval = ivl_stmt_lval(net, 0);

      sig = ivl_lval_sig(lval);
      if (sig && (ivl_signal_data_type(sig) == IVL_VT_REAL)) {
	    return show_stmt_assign_sig_real(net);
      }

      if (sig && (ivl_signal_data_type(sig) == IVL_VT_STRING)) {
	    return show_stmt_assign_sig_string(net);
      }

      if (sig && (ivl_signal_data_type(sig) == IVL_VT_DARRAY)) {
	    return show_stmt_assign_sig_darray(net);
      }

      if (sig && (ivl_signal_data_type(sig) == IVL_VT_CLASS)) {
	    return show_stmt_assign_sig_cobject(net);
      }

      return show_stmt_assign_vector(net);
}
