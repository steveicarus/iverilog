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
#ident "$Id: eval_expr.c,v 1.1 2001/03/22 05:06:21 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>

static unsigned char allocation_map[0x10000/8];

static inline int peek_bit(unsigned addr)
{
      unsigned bit = addr % 8;
      addr /= 8;
      return 1 & (allocation_map[addr] >> bit);
}

static inline void set_bit(unsigned addr)
{
      unsigned bit = addr % 8;
      addr /= 8;
      allocation_map[addr] |= (1 << bit);
}

static inline void clr_bit(unsigned addr)
{
      unsigned bit = addr % 8;
      addr /= 8;
      allocation_map[addr] &= ~(1 << bit);
}

static inline void clr_vector(struct vector_info vec)
{
      unsigned idx;
      for (idx = 0 ;  idx < vec.wid ;  idx += 1)
	    clr_bit(vec.base + idx);
}

static unsigned short allocate_vector(unsigned short wid)
{
      unsigned short base = 8;

      unsigned short idx = 0;
      while (idx < wid) {
	    assert((base + idx) < 0x10000);
	    if (peek_bit(base+idx)) {
		  base = base + idx + 1;
		  idx = 0;

	    } else {
		  idx += 1;
	    }
      }

      for (idx = 0 ;  idx < wid ;  idx += 1)
	    set_bit(base+idx);

      return base;
}

/*
 * The draw_eval_expr() function draws code to evaluate the passed
 * expression, then returns the location and width of the result.
 */

static struct vector_info draw_binary_expr(ivl_expr_t exp)
{
      struct vector_info lv = draw_eval_expr(ivl_expr_oper1(exp));
      struct vector_info rv = draw_eval_expr(ivl_expr_oper2(exp));

      switch (ivl_expr_opcode(exp)) {
	  case 'e': /* == */
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/u %u, %u, %u;\n", lv.base,
		    rv.base, lv.wid);
	    clr_vector(lv);
	    clr_vector(rv);
	    lv.base = 4;
	    lv.wid = 1;
	    break;

	  case 'n': /* != */
	    assert(lv.wid == rv.wid);
	    fprintf(vvp_out, "    %%cmp/u %u, %u, %u;\n", lv.base,
		    rv.base, lv.wid);
	    fprintf(vvp_out, "    %%inv 4, 1;\n");

	    clr_vector(lv);
	    clr_vector(rv);
	    lv.base = 4;
	    lv.wid = 1;
	    break;

	  default:
	    assert(0);
      }

      return lv;
}

static struct vector_info draw_number_expr(ivl_expr_t exp)
{
      unsigned idx;
      unsigned wid = ivl_expr_width(exp);
      const char*bits = ivl_expr_bits(exp);

      struct vector_info res;
      res.base = allocate_vector(wid);
      res.wid  = wid;

      for (idx = 0 ;  idx < wid ;  idx += 1) {
	    char src = '?';
	    switch (bits[idx]) {
		case '0':
		  src = '0';
		  break;
		case '1':
		  src = '1';
		  break;
		case 'x':
		  src = '2';
		  break;
		case 'z':
		  src = '3';
		  break;
	    }

	    fprintf(vvp_out, "    %%mov %u, %c, 1;\n", res.base+idx, src);
      }

      return res;
}

static struct vector_info draw_signal_expr(ivl_expr_t exp)
{
      unsigned idx;
      unsigned wid = ivl_expr_width(exp);
      const char*name = ivl_expr_name(exp);
      struct vector_info res;

      res.base = allocate_vector(wid);
      res.wid  = wid;

      for (idx = 0 ;  idx < wid ;  idx += 1)
	    fprintf(vvp_out, "    %%load  %u, V_%s[%u];\n",
		    res.base+idx, name, idx);

      return res;
}

struct vector_info draw_eval_expr(ivl_expr_t exp)
{
      struct vector_info res;

      switch (ivl_expr_type(exp)) {
	  case IVL_EX_NONE:
	  default:
	    assert(0);
	    res.base = 0;
	    res.wid = 0;
	    break;

	  case IVL_EX_BINARY:
	    res = draw_binary_expr(exp);
	    break;

	  case IVL_EX_NUMBER:
	    res = draw_number_expr(exp);
	    break;

	  case IVL_EX_SIGNAL:
	    res = draw_signal_expr(exp);
	    break;
      }

      return res;
}

/*
 * $Log: eval_expr.c,v $
 * Revision 1.1  2001/03/22 05:06:21  steve
 *  Geneate code for conditional statements.
 *
 */

