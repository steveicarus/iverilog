/*
 * Copyright (c) 2005-2021 Stephen Williams (steve@icarus.com)
 *
 * (This is a rewrite of code that was ...
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>)
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

#include "udp.h"
#include "schedule.h"
#include "symbols.h"
#include "compile.h"
#include "config.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#ifdef CHECK_WITH_VALGRIND
#include "vvp_cleanup.h"
#include "ivl_alloc.h"
#endif

using namespace std;

// We may need these later when we build the VPI interface to
// UDP definitions.
#ifdef CHECK_WITH_VALGRIND
static vvp_udp_s **udp_defns = 0;
static unsigned udp_defns_count = 0;

void udp_defns_delete()
{
      for (unsigned idx = 0; idx < udp_defns_count; idx += 1) {
	    if (udp_defns[idx]->is_sequential()) {
		  vvp_udp_seq_s *obj = static_cast<vvp_udp_seq_s *>
		                       (udp_defns[idx]);
		  delete obj;
	    } else {
		  vvp_udp_comb_s *obj = static_cast<vvp_udp_comb_s *>
		                        (udp_defns[idx]);
		  delete obj;
	    }
      }
      free(udp_defns);
      udp_defns = 0;
      udp_defns_count = 0;
}
#endif

static symbol_table_t udp_table;

void delete_udp_symbols()
{
      delete_symbol_table(udp_table);
      udp_table = 0;
}

struct vvp_udp_s *udp_find(const char *label)
{
      symbol_value_t v = sym_get_value(udp_table, label);
      return (struct vvp_udp_s *)v.ptr;
}

ostream& operator <<(ostream&o, const struct udp_levels_table&table)
{
      o << "[" << hex << table.mask0
	<< "/" << hex << table.mask1
	<< "/" << hex << table.maskx << "]";
      return o;
}

vvp_udp_s::vvp_udp_s(char*label, char*name__, unsigned ports,
                     vvp_bit4_t init, bool type)
: name_(name__), ports_(ports), init_(init), seq_(type)
{
      if (!udp_table)
	    udp_table = new_symbol_table();

      assert( !udp_find(label) );

      symbol_value_t v;
      v.ptr = this;
      sym_set_value(udp_table, label, v);

#ifdef CHECK_WITH_VALGRIND
      udp_defns_count += 1;
      udp_defns = (vvp_udp_s **) realloc(udp_defns,
                  udp_defns_count*sizeof(vvp_udp_s **));
      udp_defns[udp_defns_count-1] = this;
#endif
}

vvp_udp_s::~vvp_udp_s()
{
      delete[] name_;
}

unsigned vvp_udp_s::port_count() const
{
      return ports_;
}

vvp_bit4_t vvp_udp_s::get_init() const
{
      return init_;
}

vvp_udp_comb_s::vvp_udp_comb_s(char*label, char*name__, unsigned ports)
: vvp_udp_s(label, name__, ports, BIT4_X, false)
{
      levels0_ = 0;
      levels1_ = 0;
      nlevels0_ = 0;
      nlevels1_ = 0;
}

vvp_udp_comb_s::~vvp_udp_comb_s()
{
      delete[] levels0_;
      delete[] levels1_;
}

/*
 * The cur table that is passed in must have for every valid bit
 * position exactly one of the three mask bits set. This represents an
 * actual vector of inputs to be tested.
 *
 * The levels0_ and levels1_ tables have levels_table objects that
 * each represent a single row. For the row to match the input vector,
 * all the bits that are set in the cur table must also be set in the
 * row being tested.
 *
 * It is possible for a row to match multiple different vectors. This
 * is seen from the compile_table function, where bit positions for
 * multiple masks can be test for certain row positions. For example,
 * if the row bit position is '?', then mask 0/1/x are all set in the
 * row for that bit position. This means it doesn't matter which of
 * the three bit positions is set in the cur input table, the bit
 * position will generate a match.
 */
vvp_bit4_t vvp_udp_comb_s::test_levels(const udp_levels_table&cur)
{
	/* To test for a row match, test that the mask0, mask1 and
	   maskx vectors all have bits set where the matching
	   cur.mask0/1/x vectors have bits set. It is possible that a
	   levels0_[idx] vector has more bits set then the cur mask,
	   but this is OK and these bits are to be ignored. */

      for (unsigned idx = 0 ;  idx < nlevels0_ ;  idx += 1) {
	    if (cur.mask0 != (cur.mask0 & levels0_[idx].mask0))
		  continue;
	    if (cur.mask1 != (cur.mask1 & levels0_[idx].mask1))
		  continue;
	    if (cur.maskx != (cur.maskx & levels0_[idx].maskx))
		  continue;

	    return BIT4_0;
      }

      for (unsigned idx = 0 ;  idx < nlevels1_ ;  idx += 1) {
	    if (cur.mask0 != (cur.mask0 & levels1_[idx].mask0))
		  continue;
	    if (cur.mask1 != (cur.mask1 & levels1_[idx].mask1))
		  continue;
	    if (cur.maskx != (cur.maskx & levels1_[idx].maskx))
		  continue;

	    return BIT4_1;
      }

      return BIT4_X;
}

vvp_bit4_t vvp_udp_comb_s::calculate_output(const udp_levels_table&cur,
					    const udp_levels_table&,
					    vvp_bit4_t)
{
      return test_levels(cur);
}

static void or_based_on_char(udp_levels_table&cur, char flag,
			     unsigned long mask_bit)
{
      switch (flag) {
	  case '0':
	    cur.mask0 |= mask_bit;
	    break;
	  case '1':
	    cur.mask1 |= mask_bit;
	    break;
	  case 'x':
	    cur.maskx |= mask_bit;
	    break;
	  case 'b':
	    cur.mask0 |= mask_bit;
	    cur.mask1 |= mask_bit;
	    break;
	  case 'l':
	    cur.mask0 |= mask_bit;
	    cur.maskx |= mask_bit;
	    break;
	  case 'h':
	    cur.maskx |= mask_bit;
	    cur.mask1 |= mask_bit;
	    break;
	  case '?':
	    cur.mask0 |= mask_bit;
	    cur.maskx |= mask_bit;
	    cur.mask1 |= mask_bit;
	    break;
	  default:
	    fprintf(stderr, "Unsupported flag %c(%d).\n", flag, flag);
	    assert(0);
      }
}

void vvp_udp_comb_s::compile_table(char**tab)
{
      unsigned nrows0 = 0, nrows1 = 0;

	/* First run through the table to figure out the number of
	   rows I need for each kind of table. */
      for (unsigned idx = 0 ;  tab[idx] ;  idx += 1) {
	    assert(strlen(tab[idx]) == port_count() + 1);
	    switch (tab[idx][port_count()]) {
		case '0':
		  nrows0 += 1;
		  break;
		case '1':
		  nrows1 += 1;
		  break;
		case 'x':
		  break;
		default:
		  fprintf(stderr, "Unsupported entry %c(%d).\n",
		          tab[idx][port_count()], tab[idx][port_count()]);
		  assert(0);
	    }
      }

      nlevels0_ = nrows0;
      levels0_ = new udp_levels_table[nlevels0_];

      nlevels1_ = nrows1;
      levels1_ = new udp_levels_table[nlevels1_];

      nrows0 = 0;
      nrows1 = 0;
      for (unsigned idx = 0 ;  tab[idx] ;  idx += 1) {
	    struct udp_levels_table cur;
	    cur.mask0 = 0;
	    cur.mask1 = 0;
	    cur.maskx = 0;
	    if (port_count() > 8*sizeof(cur.mask0)) {
		  fprintf(stderr, "internal error: primitive port count=%u "
			  " > %zu\n", port_count(), sizeof(cur.mask0));
		  assert(port_count() <= 8*sizeof(cur.mask0));
	    }
	    for (unsigned pp = 0 ;  pp < port_count() ;  pp += 1) {
		  unsigned long mask_bit = 1UL << pp;
		  or_based_on_char(cur, tab[idx][pp], mask_bit);
	    }

	    switch (tab[idx][port_count()]) {
		case '0':
		  levels0_[nrows0++] = cur;
		  break;
		case '1':
		  levels1_[nrows1++] = cur;
		  break;
		default:
		  break;
	    }
	    delete[] tab[idx];
      }
      free(tab);

      assert(nrows0 == nlevels0_);
      assert(nrows1 == nlevels1_);
}

vvp_udp_seq_s::vvp_udp_seq_s(char*label, char*name__,
			     unsigned ports, vvp_bit4_t init)
: vvp_udp_s(label, name__, ports, init, true)
{
      levels0_ = 0;
      levels1_ = 0;
      levelsx_ = 0;
      levelsL_ = 0;

      nlevels0_ = 0;
      nlevels1_ = 0;
      nlevelsx_ = 0;
      nlevelsL_ = 0;

      edges0_ = 0;
      edges1_ = 0;
      edgesL_ = 0;

      nedges0_ = 0;
      nedges1_ = 0;
      nedgesL_ = 0;
}

vvp_udp_seq_s::~vvp_udp_seq_s()
{
      delete[] levels0_;
      delete[] levels1_;
      delete[] levelsx_;
      delete[] levelsL_;
      delete[] edges0_;
      delete[] edges1_;
      delete[] edgesL_;
}

void edge_based_on_char(struct udp_edges_table&cur, char chr, unsigned pos)
{
      unsigned long mask_bit = 1 << pos;

      switch (chr) {
	  case '0':
	    cur.mask0 |= mask_bit;
	    break;
	  case '1':
	    cur.mask1 |= mask_bit;
	    break;
	  case 'x':
	    cur.maskx |= mask_bit;
	    break;
	  case 'b':
	    cur.mask0 |= mask_bit;
	    cur.mask1 |= mask_bit;
	    break;
	  case 'l':
	    cur.mask0 |= mask_bit;
	    cur.maskx |= mask_bit;
	    break;
	  case 'h':
	    cur.maskx |= mask_bit;
	    cur.mask1 |= mask_bit;
	    break;
	  case '?':
	    cur.mask0 |= mask_bit;
	    cur.maskx |= mask_bit;
	    cur.mask1 |= mask_bit;
	    break;

	  case 'B': // (x?) edge
	    cur.mask0 |= mask_bit;
	    cur.mask1 |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 0;
	    cur.edge_maskx = 1;
	    cur.edge_mask1 = 0;
	    break;
	  case 'f': // (10) edge
	    cur.mask0 |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 0;
	    cur.edge_maskx = 0;
	    cur.edge_mask1 = 1;
	    break;
	  case 'F': // (x0) edge
	    cur.mask0 |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 0;
	    cur.edge_maskx = 1;
	    cur.edge_mask1 = 0;
	    break;
	  case 'M': // (1x) edge
	    cur.maskx |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 0;
	    cur.edge_maskx = 0;
	    cur.edge_mask1 = 1;
	    break;
	  case 'N': // (1x) and (10) edge
	    cur.mask0 |= mask_bit;
	    cur.maskx |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 0;
	    cur.edge_maskx = 0;
	    cur.edge_mask1 = 1;
	    break;
	  case 'P': // (0x) and (01) edge
	    cur.maskx |= mask_bit;
	    cur.mask1 |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 1;
	    cur.edge_maskx = 0;
	    cur.edge_mask1 = 0;
	    break;
	  case 'q': // (bx) edge
	    cur.maskx |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 1;
	    cur.edge_maskx = 0;
	    cur.edge_mask1 = 1;
	    break;
	  case 'Q': // (0x) edge
	    cur.maskx |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 1;
	    cur.edge_maskx = 0;
	    cur.edge_mask1 = 0;
	    break;
	  case 'r': // (01) edge
	    cur.mask1 |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 1;
	    cur.edge_maskx = 0;
	    cur.edge_mask1 = 0;
	    break;
	  case 'R': // (x1) edge
	    cur.mask1 |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 0;
	    cur.edge_maskx = 1;
	    cur.edge_mask1 = 0;
	    break;
	  case '%': // (?x) edge
	    cur.maskx |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 1;
	    cur.edge_maskx = 0;
	    cur.edge_mask1 = 1;
	    break;
	  case '+': // (?1) edge
	    cur.mask1 |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 1;
	    cur.edge_maskx = 1;
	    cur.edge_mask1 = 0;
	    break;
	  case '_': // (?0) edge
	    cur.mask0 |= mask_bit;
	    cur.edge_position = pos;
	    cur.edge_mask0 = 0;
	    cur.edge_maskx = 1;
	    cur.edge_mask1 = 1;
	    break;
	  default:
	    fprintf(stderr, "internal error: unknown edge code: %c\n", chr);
	    assert(0);
      }
}

void vvp_udp_seq_s::compile_table(char**tab)
{

      for (unsigned idx = 0 ;  tab[idx] ;  idx += 1) {
	    const char*row = tab[idx];
	    assert(strlen(row) == port_count() + 2);

	    if (strspn(row, "01xblh?") >= port_count()+1) {

		  switch (row[port_count()+1]) {
		      case '0':
			nlevels0_ += 1;
			break;
		      case '1':
			nlevels1_ += 1;
			break;
		      case 'x':
			nlevelsx_ += 1;
			break;
		      case '-':
			nlevelsL_ += 1;
			break;
		      default:
			fprintf(stderr, "Unsupported entry %c(%d).\n",
			        row[port_count()+1], row[port_count()+1]);
			assert(0);
			break;
		  }

	    } else {
		    /* Rows that have n or p edges will need to be
		       expanded into 2 rows. */
		  unsigned extra = 0;
		  if (strchr(row,'n'))
			extra = 1;
		  if (strchr(row,'p'))
			extra = 1;
		  if (strchr(row,'*'))
			extra = 2;

		  switch (row[port_count()+1]) {
		      case '0':
			nedges0_ += 1 + extra;
			break;
		      case '1':
			nedges1_ += 1 + extra;
			break;
		      case 'x':
			break;
		      case '-':
			nedgesL_ += 1 + extra;
			break;
		      default:
			fprintf(stderr, "Unsupported entry %c(%d).\n",
			        row[port_count()+1], row[port_count()+1]);
			assert(0);
			break;
		  }
	    }
      }

      levels0_ = new udp_levels_table[nlevels0_];
      levels1_ = new udp_levels_table[nlevels1_];
      levelsx_ = new udp_levels_table[nlevelsx_];
      levelsL_ = new udp_levels_table[nlevelsL_];
      edges0_ = new udp_edges_table[nedges0_];
      edges1_ = new udp_edges_table[nedges1_];
      edgesL_ = new udp_edges_table[nedgesL_];

      unsigned idx_lev0 = 0;
      unsigned idx_lev1 = 0;
      unsigned idx_levx = 0;
      unsigned idx_levL = 0;
      unsigned idx_edg0 = 0;
      unsigned idx_edg1 = 0;
      unsigned idx_edgL = 0;

      for (unsigned idx = 0 ;  tab[idx] ;  idx += 1) {
	    const char*row = tab[idx];

	    if (strspn(row, "01xblh?") >= port_count()+1) {
		  struct udp_levels_table cur;
		  cur.mask0 = 0;
		  cur.mask1 = 0;
		  cur.maskx = 0;
		  for (unsigned pp = 0 ;  pp < port_count() ;  pp += 1) {
			unsigned long mask_bit = 1UL << pp;
			or_based_on_char(cur, row[pp+1], mask_bit);
		  }

		  or_based_on_char(cur, row[0], 1UL << port_count());

		  switch (row[port_count()+1]) {
		      case '0':
			levels0_[idx_lev0++] = cur;
			break;
		      case '1':
			levels1_[idx_lev1++] = cur;
			break;
		      case 'x':
			levelsx_[idx_levx++] = cur;
			break;
		      case '-':
			levelsL_[idx_levL++] = cur;
			break;
		      default:
			fprintf(stderr, "Unsupported entry %c(%d).\n",
			        row[port_count()+1], row[port_count()+1]);
			assert(0);
			break;
		  }

	    } else {
		  struct udp_edges_table cur;
		  cur.mask0 = 0;
		  cur.mask1 = 0;
		  cur.maskx = 0;
		  cur.edge_position = 0;
		  cur.edge_mask0 = 0;
		  cur.edge_mask1 = 0;
		  cur.edge_maskx = 0;

		  bool need_ext0_table = false;
		  struct udp_edges_table ext0;
		  ext0.mask0 = 0;
		  ext0.mask1 = 0;
		  ext0.maskx = 0;
		  ext0.edge_position = 0;
		  ext0.edge_mask0 = 0;
		  ext0.edge_mask1 = 0;
		  ext0.edge_maskx = 0;

		  bool need_ext1_table = false;
		  struct udp_edges_table ext1;
		  ext1.mask0 = 0;
		  ext1.mask1 = 0;
		  ext1.maskx = 0;
		  ext1.edge_position = 0;
		  ext1.edge_mask0 = 0;
		  ext1.edge_mask1 = 0;
		  ext1.edge_maskx = 0;

		  for (unsigned pp = 0 ;  pp < port_count() ; pp += 1) {
			switch (row[pp+1]) {
			    case 'n':
			      edge_based_on_char(cur, 'N', pp);
			      edge_based_on_char(ext0, '_', pp);
			      need_ext0_table = true;
			      break;
			    case 'p':
			      edge_based_on_char(cur, 'P', pp);
			      edge_based_on_char(ext0, '+', pp);
			      need_ext0_table = true;
			      break;
			    case '*':
			      edge_based_on_char(cur, 'P', pp);
			      edge_based_on_char(ext0, 'N', pp);
			      edge_based_on_char(ext1, 'B', pp);
			      need_ext0_table = true;
			      need_ext1_table = true;
			      break;
			    default:
			      edge_based_on_char(cur, row[pp+1], pp);
			      edge_based_on_char(ext0, row[pp+1], pp);
			      edge_based_on_char(ext1, row[pp+1], pp);
			      break;
			}
		  }
		  edge_based_on_char(cur, row[0], port_count());
		  edge_based_on_char(ext0, row[0], port_count());
		  edge_based_on_char(ext1, row[0], port_count());

		  switch (row[port_count()+1]) {
		      case '0':
			edges0_[idx_edg0++] = cur;
			if (need_ext0_table)
			      edges0_[idx_edg0++] = ext0;
			if (need_ext1_table)
			      edges0_[idx_edg0++] = ext1;
			break;
		      case '1':
			edges1_[idx_edg1++] = cur;
			if (need_ext0_table)
			      edges1_[idx_edg1++] = ext0;
			if (need_ext1_table)
			      edges1_[idx_edg1++] = ext1;
			break;
		      case 'x':
			break;
		      case '-':
			edgesL_[idx_edgL++] = cur;
			if (need_ext0_table)
			      edgesL_[idx_edgL++] = ext0;
			if (need_ext1_table)
			      edgesL_[idx_edgL++] = ext1;
			break;
		      default:
			fprintf(stderr, "Unsupported entry %c(%d).\n",
			        row[port_count()+1], row[port_count()+1]);
			assert(0);
			break;
		  }

	    }
	    delete[] tab[idx];
      }
      free(tab);

      assert(idx_edg0 == nedges0_);
      assert(idx_edg1 == nedges1_);
      assert(idx_edgL == nedgesL_);

}

bool operator == (const udp_levels_table&a, const udp_levels_table&b)
{
      if (a.mask0 != b.mask0)
	    return false;
      if (a.mask1 != b.mask1)
	    return false;
      if (a.maskx != b.maskx)
	    return false;
      return true;
}

vvp_bit4_t vvp_udp_seq_s::calculate_output(const udp_levels_table&cur,
					   const udp_levels_table&prev,
					   vvp_bit4_t cur_out)
{
      if (cur == prev)
	    return cur_out;

      udp_levels_table cur_tmp = cur;

      unsigned long mask_out = 1UL << port_count();
      switch (cur_out) {
	  case BIT4_0:
	    cur_tmp.mask0 |= mask_out;
	    break;
	  case BIT4_1:
	    cur_tmp.mask1 |= mask_out;
	    break;
	  default:
	    cur_tmp.maskx |= mask_out;
	    break;
      }

      vvp_bit4_t lev = test_levels_(cur_tmp);
      if (lev == BIT4_Z) {
	    lev = test_edges_(cur_tmp, prev);
      }

      return lev;
}

/*
 * This function tests the levels of the input with the additional
 * check match for the current output. It uses this to calculate a
 * next output, or Z if there was no match. (This is different from
 * the combinational version of this function, which returns X for the
 * cases that don't match.) This method assumes that the caller has
 * set the mask bit in bit position [port_count()] to reflect the
 * current output.
 */
vvp_bit4_t vvp_udp_seq_s::test_levels_(const udp_levels_table&cur)
{
      for (unsigned idx = 0 ;  idx < nlevels0_ ;  idx += 1) {
	    if (cur.mask0 != (cur.mask0 & levels0_[idx].mask0))
		  continue;
	    if (cur.mask1 != (cur.mask1 & levels0_[idx].mask1))
		  continue;
	    if (cur.maskx != (cur.maskx & levels0_[idx].maskx))
		  continue;

	    return BIT4_0;
      }

      for (unsigned idx = 0 ;  idx < nlevels1_ ;  idx += 1) {
	    if (cur.mask0 != (cur.mask0 & levels1_[idx].mask0))
		  continue;
	    if (cur.mask1 != (cur.mask1 & levels1_[idx].mask1))
		  continue;
	    if (cur.maskx != (cur.maskx & levels1_[idx].maskx))
		  continue;

	    return BIT4_1;
      }

	/* We need to test against an explicit X-output table, since
	   we need to distinguish from an X output and no match. */
      for (unsigned idx = 0 ;  idx < nlevelsx_ ;  idx += 1) {
	    if (cur.mask0 != (cur.mask0 & levelsx_[idx].mask0))
		  continue;
	    if (cur.mask1 != (cur.mask1 & levelsx_[idx].mask1))
		  continue;
	    if (cur.maskx != (cur.maskx & levelsx_[idx].maskx))
		  continue;

	    return BIT4_X;
      }

	/* Test the table that requests the next output be the same as
	   the current output. This gets the current output from the
	   levels table that was passed in. */
      for (unsigned idx = 0 ;  idx < nlevelsL_ ;  idx += 1) {
	    if (cur.mask0 != (cur.mask0 & levelsL_[idx].mask0))
		  continue;
	    if (cur.mask1 != (cur.mask1 & levelsL_[idx].mask1))
		  continue;
	    if (cur.maskx != (cur.maskx & levelsL_[idx].maskx))
		  continue;

	    if (cur.mask0 & (1 << port_count()))
		  return BIT4_0;
	    if (cur.mask1 & (1 << port_count()))
		  return BIT4_1;
	    if (cur.maskx & (1 << port_count()))
		  return BIT4_X;

	    assert(0);
	    return BIT4_X;
      }

	/* No explicit levels entry match. Return a Z to signal that
	   further testing is needed. */
      return BIT4_Z;
}

vvp_bit4_t vvp_udp_seq_s::test_edges_(const udp_levels_table&cur,
				      const udp_levels_table&prev)
{
	/* The edge_mask is true for all bits that are different in
	   the cur and prev tables. */
      unsigned long edge0_mask = cur.mask0 ^ prev.mask0;
      unsigned long edgex_mask = cur.maskx ^ prev.maskx;
      unsigned long edge1_mask = cur.mask1 ^ prev.mask1;

      unsigned long edge_mask = edge0_mask|edgex_mask|edge1_mask;
      edge_mask &= ~ (-1UL << port_count());

	/* If there are no differences, then there are no edges. Give
	   up now. */
      if (edge_mask == 0)
	    return BIT4_X;

      unsigned edge_position = 0;
      while ((edge_mask&1) == 0) {
	    edge_mask >>= 1;
	    edge_position += 1;
      }

	/* We expect that there is exactly one edge in here. */
      assert(edge_mask == 1);

      edge_mask = 1UL << edge_position;

      unsigned edge_mask0 = (prev.mask0&edge_mask)? 1 : 0;
      unsigned edge_maskx = (prev.maskx&edge_mask)? 1 : 0;
      unsigned edge_mask1 = (prev.mask1&edge_mask)? 1 : 0;


	/* Now the edge_position and edge_mask* variables have the
	   values we use to test the applicability of the edge_table
	   entries. */

      for (unsigned idx = 0 ;  idx < nedges0_ ;  idx += 1) {
	    struct udp_edges_table*row = edges0_ + idx;

	    if (row->edge_position != edge_position)
		  continue;
	    if (edge_mask0 && !row->edge_mask0)
		  continue;
	    if (edge_maskx && !row->edge_maskx)
		  continue;
	    if (edge_mask1 && !row->edge_mask1)
		  continue;
	    if (cur.mask0 != (cur.mask0 & row->mask0))
		  continue;
	    if (cur.maskx != (cur.maskx & row->maskx))
		  continue;
	    if (cur.mask1 != (cur.mask1 & row->mask1))
		  continue;

	    return BIT4_0;
      }

      for (unsigned idx = 0 ;  idx < nedges1_ ;  idx += 1) {
	    struct udp_edges_table*row = edges1_ + idx;

	    if (row->edge_position != edge_position)
		  continue;
	    if (edge_mask0 && !row->edge_mask0)
		  continue;
	    if (edge_maskx && !row->edge_maskx)
		  continue;
	    if (edge_mask1 && !row->edge_mask1)
		  continue;
	    if (cur.mask0 != (cur.mask0 & row->mask0))
		  continue;
	    if (cur.maskx != (cur.maskx & row->maskx))
		  continue;
	    if (cur.mask1 != (cur.mask1 & row->mask1))
		  continue;

	    return BIT4_1;
      }

      for (unsigned idx = 0 ;  idx < nedgesL_ ;  idx += 1) {
	    struct udp_edges_table*row = edgesL_ + idx;

	    if (row->edge_position != edge_position)
		  continue;
	    if (edge_mask0 && !row->edge_mask0)
		  continue;
	    if (edge_maskx && !row->edge_maskx)
		  continue;
	    if (edge_mask1 && !row->edge_mask1)
		  continue;
	    if (cur.mask0 != (cur.mask0 & row->mask0))
		  continue;
	    if (cur.maskx != (cur.maskx & row->maskx))
		  continue;
	    if (cur.mask1 != (cur.mask1 & row->mask1))
		  continue;

	    if (cur.mask0 & (1 << port_count()))
		  return BIT4_0;
	    if (cur.mask1 & (1 << port_count()))
		  return BIT4_1;
	    if (cur.maskx & (1 << port_count()))
		  return BIT4_X;

	    assert(0);
	    return BIT4_X;
      }

      return BIT4_X;
}

vvp_udp_fun_core::vvp_udp_fun_core(vvp_net_t*net, vvp_udp_s*def)
: vvp_wide_fun_core(net, def->port_count())
{
      def_ = def;
      cur_out_ = def_->get_init();
	// Assume initially that all the inputs are 1'bx
      current_.mask0 = 0;
      current_.mask1 = 0;
      current_.maskx = ~ ((-1UL) << port_count());

        // If the initial value is 0 or 1, schedule the initial assignment
        // normally, so that any sensitive always processes can be started
        // first.
      if (cur_out_ != BIT4_X)
	    schedule_generic(this, 0, false);
      else
	    schedule_functor(this);
}

vvp_udp_fun_core::~vvp_udp_fun_core()
{
}

/*
 * This method is used to propagate the initial value on startup.
 */
void vvp_udp_fun_core::run_run()
{
      vvp_vector4_t tmp (1);
      tmp.set_bit(0, cur_out_);
      propagate_vec4(tmp);
}

void vvp_udp_fun_core::recv_vec4_from_inputs(unsigned port)
{
	/* For now, assume udps are 1-bit wide. */
      assert(value(port).size() == 1);

      unsigned long mask = 1UL << port;

      udp_levels_table prev = current_;

      switch (value(port).value(0)) {

	  case BIT4_0:
	    current_.mask0 |= mask;
	    current_.mask1 &= ~mask;
	    current_.maskx &= ~mask;
	    break;
	  case BIT4_1:
	    current_.mask0 &= ~mask;
	    current_.mask1 |= mask;
	    current_.maskx &= ~mask;
	    break;
	  default:
	    current_.mask0 &= ~mask;
	    current_.mask1 &= ~mask;
	    current_.maskx |= mask;
	    break;
      }

      vvp_bit4_t out_bit = def_->calculate_output(current_, prev, cur_out_);

      if (out_bit == cur_out_)
	    return;

      cur_out_ = out_bit;
      schedule_functor(this);
}


/*
 * This function is called by the parser in response to a .udp
 * node. We create the nodes needed to integrate the UDP into the
 * netlist. The definition should be parsed already.
 */
void compile_udp_functor(char*label, char*type,
			 unsigned argc, struct symb_s*argv)
{
      struct vvp_udp_s *def = udp_find(type);
      assert(def);
      free(type);

      vvp_net_t*ptr = new vvp_net_t;
      vvp_udp_fun_core*core = new vvp_udp_fun_core(ptr, def);
      ptr->fun = core;

      define_functor_symbol(label, ptr);
      free(label);

      wide_inputs_connect(core, argc, argv);
      free(argv);
}
