/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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
      unsigned   bit;
      unsigned   alloc : 1;
} allocation_map[MAX_VEC] = { {0} };

/* This is the largest bit to have lookaside values. */
static unsigned lookaside_top = 0;

static inline int peek_bit(unsigned addr)
{
      return allocation_map[addr].alloc;
}

static inline void set_bit(unsigned addr)
{
      allocation_map[addr].alloc = 1;
}

static inline void clr_bit(unsigned addr)
{
      allocation_map[addr].alloc = 0;
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
      for (idx = 0 ;  idx < vec.wid ;  idx += 1)
	    clr_bit(vec.base + idx);
}

/*
 * This unconditionally allocates a stretch of bits from the register
 * set. It never returns a bit addressed <8 (0-3 are constant, 4-7 are
 * condition codes).
 */
unsigned allocate_vector(unsigned wid)
{
      unsigned base = 8;
      unsigned idx = 0;

      while (idx < wid) {
	    assert((base + idx) < MAX_VEC);
	    if (peek_bit(base+idx)) {
		  base = base + idx + 1;
		  idx = 0;

	    } else {
		  idx += 1;
	    }
      }

      for (idx = 0 ;  idx < wid ;  idx += 1) {
	    allocation_map[base+idx].alloc = 1;
	    allocation_map[base+idx].exp = 0;
      }

      return base;
}

/*
 * This clears the expression cache of the allocation map. It is
 * called to prevent reuse of existing expressions, normally at the
 * start of a basic block.
 */
void clear_expression_lookaside(void)
{
      unsigned idx;
      for (idx = 0 ;  idx < lookaside_top ;  idx += 1) {
	    allocation_map[idx].exp = 0;
      }

      lookaside_top = 0;
}

void save_expression_lookaside(unsigned addr, ivl_expr_t exp,
			       unsigned wid)
{
      unsigned idx;
      assert(addr >= 8);
      assert((addr+wid) <= MAX_VEC);

      for (idx = 0 ;  idx < wid ;  idx += 1) {
	    allocation_map[addr+idx].exp = exp;
	    allocation_map[addr+idx].bit = idx;
      }

      if ((addr+wid) > lookaside_top)
	    lookaside_top = addr+wid;
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
	  case IVL_EX_SIGNAL:
	    if (ivl_expr_signal(l) != ivl_expr_signal(r))
		  return 0;

	    if (ivl_expr_lsi(l) != ivl_expr_lsi(r))
		  return 0;

	    if (ivl_expr_width(l) != ivl_expr_width(r))
		  return 0;

	    return 1;

	  default:
	    break;
      }

      return 0;
}

static unsigned find_expression_lookaside(ivl_expr_t exp,
						unsigned wid)
{
      unsigned top;
      unsigned idx, match;

      if (lookaside_top <= wid)
	    return 0;

      top = lookaside_top - wid + 1;

      assert(exp);
      match = 0;
      for (idx = 8 ;  idx < top ;  idx += 1) {
	    if (! compare_exp(allocation_map[idx].exp, exp)) {
		  match = 0;
		  continue;
	    }

	    if (allocation_map[idx].bit != match) {
		  match = 0;
		  continue;
	    }

	    match += 1;
	    if (match == wid)
		  return idx-match+1;
      }

      return 0;
}

unsigned allocate_vector_exp(ivl_expr_t exp, unsigned wid)
{
      unsigned idx;
      unsigned la = find_expression_lookaside(exp, wid);

      for (idx = 0 ;  idx < wid ;  idx += 1)
	    if (allocation_map[la+idx].alloc)
		  return 0;

      for (idx = 0 ;  idx < wid ;  idx += 1)
	    allocation_map[la+idx].alloc = 1;

      return la;
}
