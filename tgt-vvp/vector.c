/*
 * Copyright (c) 2002-2011 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
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
# include  <assert.h>

/* Maximum vector bits in a thread. If a thread co-processor is
 * implemented, this value may need to be reduced. At that time
 * wider operations will need to be partitioned. For example
 * shift operations on WIDE (say > 64k bit) registers.
 */
#define MAX_VEC	(256*1024)

static struct allocation_score_s {
      ivl_expr_t exp;
      ivl_signal_t sig;
      unsigned sig_word;
      unsigned  exp_bit : 24;
      unsigned  sig_bit : 24;
      unsigned alloc    :  8;
} allocation_map[MAX_VEC] = { {0} };

/* This is the largest bit to have lookaside values. */
static unsigned lookaside_top = 0;

static __inline__ ivl_expr_t peek_exp(unsigned addr)
{
      return allocation_map[addr].exp;
}

static __inline__ unsigned peek_exp_bit(unsigned addr)
{
      return allocation_map[addr].exp_bit;
}

static __inline__ void set_exp(unsigned addr, ivl_expr_t expr, unsigned ebit)
{
      allocation_map[addr].exp = expr;
      allocation_map[addr].exp_bit = ebit;
}

static __inline__ void set_sig(unsigned addr, ivl_signal_t expr,
                               unsigned sig_word, unsigned ebit)
{
      allocation_map[addr].sig = expr;
      allocation_map[addr].sig_word = sig_word;
      allocation_map[addr].sig_bit = ebit;
}

/*
 * This clears a vector that was previously allocated by
 * allocate_vector. That is, it unmarks all the bits of the map that
 * represent this vector.
 *
 * If the vector is based in one of 4 constant bit values, then there
 * are no bits to clear. If the vector is based in the 4-8 result
 * area, then someone is broken.
 */
void clr_vector(struct vector_info vec)
{
      unsigned idx;
      if (vec.base < 4)
	    return;
      assert(vec.base >= 8);
      for (idx = 0 ;  idx < vec.wid ;  idx += 1) {
	    assert( allocation_map[vec.base+idx].alloc > 0);
	    allocation_map[vec.base+idx].alloc -= 1;
      }
}

static unsigned allocate_vector_no_lookaside(unsigned wid, int skip_lookaside)
{
      unsigned base = 8;
      unsigned idx = 0;

      while (idx < wid) {
	    if (base+idx >= MAX_VEC)
		  return 0;

	    assert((base + idx) < MAX_VEC);
	    if ((allocation_map[base+idx].alloc > 0)
		|| (skip_lookaside && peek_exp(base+idx))) {
		  base = base + idx + 1;
		  idx = 0;

	    } else {
		  idx += 1;
	    }
      }

      for (idx = 0 ;  idx < wid ;  idx += 1) {
	    allocation_map[base+idx].alloc += 1;
	    set_exp(base+idx, 0, 0);
	    set_sig(base+idx, 0, 0, 0);
      }

      return base;
}

/*
 * This unconditionally allocates a stretch of bits from the register
 * set. It never returns a bit addressed <8 (0-3 are constant, 4-7 are
 * condition codes).
 *
 * First try to allocate a vector without interfering with any bits
 * cached by the lookaside buffer. If that doesn't work, then try
 * again without worrying about trashing lookaside results. This
 * should lead to preferentially allocating new bits instead of
 * constantly overwriting intermediate results.
 *
 * If there is no space for a vector of the given width, then give up
 * and return 0.
 */
unsigned allocate_vector(unsigned wid)
{
      unsigned base = allocate_vector_no_lookaside(wid, 1);

      if (base == 0)
	    base = allocate_vector_no_lookaside(wid, 0);
      return base;
}

/*
 * This clears the expression cache of the allocation map. It is
 * called to prevent reuse of existing expressions, normally at the
 * start of a basic block, but also at the end of thread processing.
 */
void clear_expression_lookaside(void)
{
      unsigned idx;

      for (idx = 0 ;  idx < lookaside_top ;  idx += 1) {
	    set_exp(idx, 0, 0);
	    set_sig(idx, 0, 0, 0);
      }

      lookaside_top = 0;
}

static int test_expression_savable(ivl_expr_t expr)
{
      switch (ivl_expr_type(expr)) {

	  case IVL_EX_NUMBER:
	  case IVL_EX_STRING:
	    return 1;

	  default:
	    return 0;
      }
}

void save_expression_lookaside(unsigned addr, ivl_expr_t expr, unsigned wid)
{
      unsigned idx;
      assert(addr >= 8);
      assert((addr+wid) <= MAX_VEC);

	/* When saving an expression to the lookaside, also clear the
	   signal saved in the lookaside for these bits. The reason is
	   that an expression calculation will replace any signal
	   bits. */
      for (idx = 0 ;  idx < wid ;  idx += 1)
	    set_sig(addr+idx, 0, 0, 0);

	/* Only certain types of expressions are savable. */
      if ( ! test_expression_savable(expr))
	    return;

      for (idx = 0 ;  idx < wid ;  idx += 1) {
	    set_exp(addr+idx, expr, idx);
      }

      if ((addr+wid) > lookaside_top)
	    lookaside_top = addr+wid;
}

static void clear_signal_lookaside_bit(unsigned idx, ivl_signal_t sig, unsigned sig_word)
{
      if (allocation_map[idx].alloc > 0)
	    return;
      if (allocation_map[idx].sig != sig)
	    return;
      if (allocation_map[idx].sig_word != sig_word)
	    return;

      set_sig(idx, 0, 0, 0);
}

void save_signal_lookaside(unsigned addr, ivl_signal_t sig, unsigned sig_word, unsigned wid)
{
      unsigned idx;
	/* Don't bind any of the low bits to a signal. */
      if (addr < 8 && wid > 0)
	    return;

      assert((addr+wid) <= MAX_VEC);

      for (idx = 8 ;  idx < addr ;  idx += 1)
	    clear_signal_lookaside_bit(idx, sig, sig_word);

      for (idx = 0 ;  idx < wid ;  idx += 1)
	    set_sig(addr+idx, sig, sig_word, idx);

      if ((addr+wid) > lookaside_top)
	    lookaside_top = addr+wid;

      for (idx = addr+wid ;  idx < lookaside_top ;  idx += 1)
	    clear_signal_lookaside_bit(idx, sig, sig_word);
}

static int compare_exp(ivl_expr_t l, ivl_expr_t r)
{
      if (! (l && r))
	    return 0;
      if (l == r)
	    return 1;

      if (ivl_expr_type(l) != ivl_expr_type(r))
	    return 0;

      switch (ivl_expr_type(l)) {

	  case IVL_EX_NUMBER:
	    if (ivl_expr_width(l) != ivl_expr_width(r))
		  return 0;
	    { const char*bitl = ivl_expr_bits(l);
	      const char*bitr = ivl_expr_bits(r);
	      unsigned idx;
	      for (idx = 0 ;  idx < ivl_expr_width(l) ;  idx += 1) {
		    if (bitl[idx] != bitr[idx])
			  return 0;
	      }
	    }
	    return 1;

	  case IVL_EX_SELECT:
	    if (! compare_exp(ivl_expr_oper1(l), ivl_expr_oper1(r)))
		  return 0;

	    if (ivl_expr_oper2(l) == 0 && ivl_expr_oper1(r) == 0)
		  return 1;

	    if (! compare_exp(ivl_expr_oper2(l), ivl_expr_oper2(r)))
		  return 0;

	    return 1;

	  case IVL_EX_SIGNAL:
	    if (ivl_expr_signal(l) != ivl_expr_signal(r))
		  return 0;

	    if (ivl_expr_width(l) != ivl_expr_width(r))
		  return 0;

	      /* Don't match array words. */
	    if (ivl_expr_oper1(l) || ivl_expr_oper1(r))
		  return 0;

	    return 1;

	  default:
	    break;
      }

      return 0;
}

static unsigned find_expression_lookaside(ivl_expr_t expr, unsigned wid)
{
      unsigned idx, match;
      ivl_signal_t sig;

      if (lookaside_top <= wid)
	    return 0;

	/* Look in the expression lookaside for this expression. */
      assert(expr);
      match = 0;
      for (idx = 8 ;  idx < lookaside_top ;  idx += 1) {
	    if (! compare_exp(allocation_map[idx].exp, expr)) {
		  match = 0;
		  continue;
	    }

	    if (allocation_map[idx].exp_bit != match) {
		  match = 0;
		  continue;
	    }

	    match += 1;
	    if (match == wid)
		  return idx-match+1;
      }

	/* The general expression lookup failed. If this is an
	   IVL_EX_SIGNAL, then look again in the variable lookaside
	   (which is saved l-values) for the expression. */
      if (ivl_expr_type(expr) != IVL_EX_SIGNAL)
	    return 0;

      sig = ivl_expr_signal(expr);

	/* Only reg signals (variables) will be in the signal
	   lookaside, because only blocking assigned values are in the
	   signal lookaside. */
      if (ivl_signal_type(sig) != IVL_SIT_REG)
	    return 0;

	/* Now look for signal value matches in the signal lookaside. */
      match = 0;
      for (idx = 8 ;  idx < lookaside_top ;  idx += 1) {
	    if (sig != allocation_map[idx].sig) {
		  match = 0;
		  continue;
	    }

	    if (allocation_map[idx].sig_bit != match) {
		  match = 0;
		  continue;
	    }

	    match += 1;
	    if (match == wid)
		  return idx-match+1;

      }

      return 0;
}

/*
 * Look for the expression in the expression lookaside table. If it is
 * there, then allocate it and return the base. In this case the
 * caller will not need to evaluate the expression. If this function
 * returns 0, then the expression is not found and nothing is allocated.
 */
unsigned allocate_vector_exp(ivl_expr_t expr, unsigned wid,
			     int exclusive_flag)
{
      unsigned idx;
      unsigned la = find_expression_lookaside(expr, wid);
      if (la == 0)
	    return 0;

      if (exclusive_flag) {
	      /* If the caller is requesting exclusive allocation of
	         the expression, then return not-found if a lookup
	         already matched the expression. */
	    for (idx = 0 ;  idx < wid ;  idx += 1)
		  if (allocation_map[la+idx].alloc)
			return 0;
      }

      for (idx = 0 ;  idx < wid ;  idx += 1)
	    allocation_map[la+idx].alloc += 1;

      return la;
}

